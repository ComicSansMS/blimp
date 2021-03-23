#include <db/blimpdb.hpp>
#include <db/table/table_layout.hpp>
#include <db/table/blimp_properties.hpp>
#include <db/table/file_contents.hpp>
#include <db/table/file_element.hpp>
#include <db/table/indexed_locations.hpp>
#include <db/table/user_selection.hpp>
#include <db/table/snapshot.hpp>
#include <db/table/snapshot_contents.hpp>
#include <db/table/sqlite_master.hpp>
#include <db/table/storage.hpp>
#include <db/table/storage_contents.hpp>

#include <exceptions.hpp>
#include <version.hpp>

#include <gbBase/Assert.hpp>
#include <gbBase/Log.hpp>

#include <sqlpp11/sqlpp11.h>
#include <sqlpp11/sqlite3/sqlite3.h>

#include <boost/variant.hpp>

#include <cstdint>
#include <limits>
#include <ostream>

namespace
{
inline bool constexpr sqlpp11_debug()
{
#ifdef NDEBUG
    return false;
#else
    return true;
#endif
}

std::string to_string(boost::filesystem::path const& p)
{
    /// @todo: proper unicode suppport
    return p.generic_string();
}
}

struct BlimpDB::Pimpl
{
    sqlpp::sqlite3::connection db;

    struct prepared_statements {

    };

    Pimpl(sqlpp::sqlite3::connection_config const& conf);
};

BlimpDB::Pimpl::Pimpl(sqlpp::sqlite3::connection_config const& conf)
    :db(conf)
{
}

BlimpDB::BlimpDB(std::string const& db_filename, OpenMode mode)
    :m_pimpl(nullptr)
{
    if(mode == OpenMode::CreateNew) {
        createNewFileDatabase(db_filename);
    } else if(mode == OpenMode::OpenExisting) {
        openExistingFileDatabase(db_filename);
    }
    m_pimpl->db.execute("PRAGMA locking_mode = EXCLUSIVE");
}

BlimpDB::~BlimpDB() = default;      // needed for pimpl destruction

void createBlimpPropertiesTable(sqlpp::sqlite3::connection& db)
{
    auto const prop_tab = blimpdb::BlimpProperties{};
    db.execute(blimpdb::table_layout::blimp_properties());
    db.execute(blimpdb::table_layout::user_selection());
    db.execute(blimpdb::table_layout::indexed_locations());
    db.execute(blimpdb::table_layout::file_contents());
    db.execute(blimpdb::table_layout::file_element());
    db.execute(blimpdb::table_layout::snapshot());
    db.execute(blimpdb::table_layout::snapshot_contents());
    db.execute(blimpdb::table_layout::storage());
    db.execute(blimpdb::table_layout::storage_contents());

    db.execute("CREATE UNIQUE INDEX idx_indexed_locations_paths ON indexed_locations (path);");
    db.execute("CREATE INDEX idx_file_element_locations ON file_element (location_id);");
    db.execute("CREATE UNIQUE INDEX idx_file_content_hashes ON file_contents (hash);");

    db(insert_into(prop_tab).set(prop_tab.id    = "version",
                                 prop_tab.value = std::to_string(BlimpVersion::version())));
}

void BlimpDB::createNewFileDatabase(std::string const& db_filename)
{
    GHULBUS_LOG(Info, "Creating new database at " << db_filename << ".");

    sqlpp::sqlite3::connection_config conf;
    conf.debug = sqlpp11_debug();
    conf.flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;
    conf.path_to_database = db_filename;

    GHULBUS_ASSERT_PRD(!m_pimpl);
    m_pimpl = std::make_unique<Pimpl>(conf);
    createBlimpPropertiesTable(m_pimpl->db);

    GHULBUS_LOG(Info, " Successfully established database at " << db_filename << ".");
}

void BlimpDB::openExistingFileDatabase(std::string const& db_filename)
{
    sqlpp::sqlite3::connection_config conf;
    conf.debug = sqlpp11_debug();
    conf.flags = SQLITE_OPEN_READWRITE;
    conf.path_to_database = db_filename;

    GHULBUS_ASSERT_PRD(!m_pimpl);
    m_pimpl = std::make_unique<Pimpl>(conf);
    auto& db = m_pimpl->db;

    auto const master_tab = blimpdb::SqliteMaster{};
    if(db(select(master_tab.name)
        .from(master_tab)
        .where((master_tab.name == "blimp_properties") && (master_tab.type == "table"))).empty())
    {
        GHULBUS_THROW(Exceptions::DatabaseError(), "Database file does not contain a blimp properties table.");
    }

    auto const prop_tab = blimpdb::BlimpProperties{};
    for(auto const& r : db(select(prop_tab.value).from(prop_tab).where(prop_tab.id == "version")))
    {
        int const version = std::stoi(r.value);
        GHULBUS_LOG(Trace, "Database version " << version);
        if(version != BlimpVersion::version())
        {
            GHULBUS_THROW(Exceptions::DatabaseError(), "Unsupported database version " + std::string(r.value) + ".");
        }
    }
}

void BlimpDB::setUserSelection(std::vector<std::string> const& selected_files)
{
    GHULBUS_LOG(Debug, "Updating user selection with " << selected_files.size() <<
                        " entr" << ((selected_files.size() == 1) ? "y" : "ies") << ".");
    auto const tab = blimpdb::UserSelection{};
    auto& db = m_pimpl->db;
    db(remove_from(tab).unconditionally());
    for(auto const& f : selected_files) {
        db(insert_into(tab).set(tab.path = f));
    }
}

std::vector<std::string> BlimpDB::getUserSelection()
{
    std::vector<std::string> ret;
    auto const tab = blimpdb::UserSelection{};
    auto& db = m_pimpl->db;
    auto const row_count = db(select(count(tab.path)).from(tab).unconditionally()).begin()->count;
    ret.reserve(row_count);
    for(auto const& r : db(select(all_of(tab)).from(tab).unconditionally())) {
        ret.push_back(r.path);
    }
    return ret;
}

FileIndexDiff BlimpDB::compareFileIndex(std::vector<FileInfo> const& fresh_index)
{
    // look up file_element for each element in fresh index
    // -> each item is either unchanged, new or updated
    // find elements from last snapshot not in fresh index
    // -> deleted
    auto const tab_loc = blimpdb::IndexedLocations{};
    auto const tab_fel = blimpdb::FileElement{};
    auto& db = m_pimpl->db;
    auto const q_find_elements_param =
        select(tab_fel.fileId, tab_fel.fileSize, tab_fel.modifiedDate)
            .from(tab_fel.join(tab_loc).on(tab_fel.locationId == select(tab_loc.locationId)))
            .where(tab_loc.path == parameter(tab_loc.path));
    auto q_find_elements_prepped = db.prepare(q_find_elements_param);

    FileIndexDiff diff;
    diff.index_files.reserve(fresh_index.size());
    for(auto const& finfo : fresh_index) {
        auto const path_string = finfo.path.generic_string();
        q_find_elements_prepped.params.path = path_string;
        auto rows = db(q_find_elements_prepped);
        auto const element_diff = [&]() -> FileIndexDiff::ElementDiff
            {
                FileIndexDiff::ElementDiff ret;
                if(rows.empty()) {
                    ret.sync_status = FileSyncStatus::NewFile;
                    ret.reference_db_id = -1;
                    ret.reference_size = 0;
                    ret.reference_modified_time = std::chrono::system_clock::time_point();
                    return ret;
                } else {
                    std::chrono::time_point<std::chrono::system_clock, std::chrono::microseconds> newest_time;
                    for(auto const& r : rows) {
                        std::uintmax_t const fsize = r.fileSize;
                        std::chrono::time_point<std::chrono::system_clock, std::chrono::microseconds> const ftime
                            = r.modifiedDate;
                        if(fsize == finfo.size && ftime == finfo.modified_time) {
                            ret.sync_status = FileSyncStatus::Unchanged;
                            ret.reference_db_id = r.fileId;
                            ret.reference_size = r.fileSize;
                            ret.reference_modified_time = ftime;
                            return ret;
                        }
                        if(ftime > newest_time) {
                            ret.reference_db_id = r.fileId;
                            ret.reference_size = r.fileSize;
                            ret.reference_modified_time = ftime;
                        }
                    }
                    ret.sync_status = FileSyncStatus::FileChanged;
                    return ret;
                }
            }();
        if(element_diff.sync_status == FileSyncStatus::NewFile) {
            GHULBUS_LOG(Trace, "New file " << path_string);
            
        } else if(element_diff.sync_status == FileSyncStatus::Unchanged) {
            GHULBUS_ASSERT(element_diff.reference_db_id != -1);
            GHULBUS_LOG(Trace, "Unchanged file " << element_diff.reference_db_id << " " << path_string);
        } else {
            GHULBUS_ASSERT(element_diff.sync_status == FileSyncStatus::FileChanged);
            GHULBUS_ASSERT(element_diff.reference_db_id != -1);
            GHULBUS_LOG(Trace, "File changed " << element_diff.reference_db_id << " " << path_string);
        }
        
        diff.index_files.push_back(element_diff);
    }
    return diff;
}

std::vector<BlimpDB::FileIndexInfo> BlimpDB::updateFileIndex(std::vector<FileInfo> const& fresh_index,
                                                             std::vector<Hash> const& hashes)
{
    GHULBUS_PRECONDITION(fresh_index.size() == hashes.size());
    std::vector<FileIndexInfo> ret;
    ret.reserve(fresh_index.size());
    auto const tab_loc = blimpdb::IndexedLocations{};
    auto const tab_fco = blimpdb::FileContents{};
    auto const tab_fel = blimpdb::FileElement{};
    auto& db = m_pimpl->db;
    db.execute("PRAGMA synchronous = OFF");
    db.start_transaction();
    auto q_find_loc_param = select(tab_loc.locationId).from(tab_loc).where(tab_loc.path == parameter(tab_loc.path));
    auto q_find_loc_prepped = db.prepare(q_find_loc_param);
    auto q_insert_loc_param = insert_into(tab_loc).set(tab_loc.path = parameter(tab_loc.path));
    auto q_insert_loc_prepped = db.prepare(q_insert_loc_param);
    auto q_find_fco_param = select(tab_fco.contentId).from(tab_fco)
                                                     .where(tab_fco.hash     == parameter(tab_fco.hash));
    auto q_find_fco_prepped = db.prepare(q_find_fco_param);
    auto q_insert_fco_param = insert_into(tab_fco).set(tab_fco.hash     = parameter(tab_fco.hash));
    auto q_insert_fco_prepped = db.prepare(q_insert_fco_param);
    auto q_find_fel_param = select(all_of(tab_fel)).from(tab_fel)
                                                   .where(tab_fel.locationId == parameter(tab_fel.locationId));
    auto q_find_fel_prepped = db.prepare(q_find_fel_param);
    auto q_insert_fel_param = insert_into(tab_fel).set(tab_fel.locationId   = parameter(tab_fel.locationId),
                                                       tab_fel.contentId    = parameter(tab_fel.contentId),
                                                       tab_fel.fileSize     = parameter(tab_fel.fileSize),
                                                       tab_fel.modifiedDate = parameter(tab_fel.modifiedDate));
    auto q_insert_fel_prepped = db.prepare(q_insert_fel_param);

    for(std::size_t i = 0; i < fresh_index.size(); ++i) {
        auto const& finfo = fresh_index[i];
        std::string const fhash = to_string(hashes[i]);
        FileSyncStatus sync_status = FileSyncStatus::Unchanged;
        auto const path_string = finfo.path.generic_string();
        q_find_loc_prepped.params.path = path_string;

        auto const location_row = db(q_find_loc_prepped);
        std::size_t location_id;
        if(!location_row.empty()) {
            location_id = location_row.front().locationId;
        } else {
            // first time we see this location; add an entry to indexed_locations
            sync_status = FileSyncStatus::NewFile;
            q_insert_loc_prepped.params.path = path_string;
            location_id = db(q_insert_loc_prepped);
        }

        if(sync_status != FileSyncStatus::NewFile)
        {
            // todo: sync status could be Unchanged or FileChanged
            q_find_fel_prepped.params.locationId = location_id;
            for(auto const& fel_row : db(q_find_fel_prepped)) {
                auto ts = fel_row.modifiedDate;
                if(finfo.size != static_cast<std::uintmax_t>(fel_row.fileSize)) {
                }
            }
        }
        ret.emplace_back(location_id, sync_status);

        q_find_fco_prepped.params.hash = fhash;
        auto const hash_row = db(q_find_fco_prepped);
        std::size_t content_id;
        if(!hash_row.empty()) {
            content_id = hash_row.front().contentId;
        } else {
            // first time we see this hash; add an entry to file_contents
            q_insert_fco_prepped.params.hash = fhash;
            content_id = db(q_insert_fco_prepped);
        }

        q_insert_fel_prepped.params.locationId   = location_id;
        q_insert_fel_prepped.params.contentId    = content_id;
        q_insert_fel_prepped.params.fileSize     = static_cast<int64_t>(finfo.size);
        auto const casted_tp = std::chrono::time_point_cast<std::chrono::microseconds>(finfo.modified_time);
        q_insert_fel_prepped.params.modifiedDate = casted_tp;
        db(q_insert_fel_prepped);
    }
    db.commit_transaction();
    db.execute("PRAGMA synchronous = FULL");

    return ret;
}

std::vector<BlimpDB::SnapshotInfo> BlimpDB::getSnapshots()
{
    std::vector<SnapshotInfo> ret;

    auto const tab_snapshot = blimpdb::Snapshot{};

    auto& db = m_pimpl->db;

    auto q_select_snapshots = select(tab_snapshot.snapshotId, tab_snapshot.name, tab_snapshot.date)
                                .from(tab_snapshot)
                                .unconditionally();

    for (auto const& r : db(q_select_snapshots)) {
        auto const d = r.date.value();
        ret.emplace_back();
        ret.back().id.i = r.snapshotId;
        ret.back().name = r.name;
        sqlpp::chrono::day_point const ddp = r.date;
        ret.back().date = ddp;
    }

    return ret;
}

BlimpDB::SnapshotId BlimpDB::addSnapshot(std::string const& name)
{
    auto& db = m_pimpl->db;
    auto const tab_snapshot = blimpdb::Snapshot{};
    auto const r =
    db(insert_into(tab_snapshot).set(tab_snapshot.name = name,
                                     tab_snapshot.date = date::floor<sqlpp::chrono::days>(std::chrono::system_clock::now())));

    return SnapshotId{ static_cast<int64_t>(r) };
}

BlimpDB::FileElementId BlimpDB::newFileContent(FileInfo const& finfo, Hash const& hash, bool do_sync)
{
    auto& db = m_pimpl->db;
    auto const tab_file_contents = blimpdb::FileContents{};
    FileContentId content_id;
    auto const hash_str = to_string(hash);
    auto const result_content = db(select(tab_file_contents.contentId)
                                   .from(tab_file_contents)
                                   .where(tab_file_contents.hash == hash_str));
    if (do_sync) { db.start_transaction(); }
    if (!result_content.empty())
    {
        auto const& r = result_content.front();
        content_id = FileContentId{ .i = r.contentId };
        GHULBUS_LOG(Debug, "Storing file element for " << finfo.path << " under existing content id " << content_id.i);
    } else {
        content_id = FileContentId{ .i =
            static_cast<int64_t>(db(insert_into(tab_file_contents).set(tab_file_contents.hash = hash_str))) };
        GHULBUS_LOG(Debug, "Storing file element for " << finfo.path << " under new content id " << content_id.i);
    }
    FileElementId const ret = newFileElement(finfo, content_id, false);
    if (do_sync) { db.commit_transaction(); }
    return ret;
}

BlimpDB::FileElementId BlimpDB::newFileElement(FileInfo const& finfo, FileContentId const& content_id, bool do_sync)
{
    auto& db = m_pimpl->db;
    auto const tab_indexed_locations = blimpdb::IndexedLocations{};
    auto const tab_file_element = blimpdb::FileElement{};

    auto const result_location = db(select(tab_indexed_locations.locationId)
                                    .from(tab_indexed_locations)
                                    .where(tab_indexed_locations.path == to_string(finfo.path)));
    if (result_location.empty()) {
        // first time we've seen this file; add a new location and file element
        if (do_sync) { db.start_transaction(); }
        int64_t const location_id =
            db(insert_into(tab_indexed_locations).set(tab_indexed_locations.path = to_string(finfo.path)));
        int64_t const file_element_id =
            db(insert_into(tab_file_element).set(tab_file_element.locationId = location_id,
                                                 tab_file_element.contentId = content_id.i,
                                                 tab_file_element.fileSize = finfo.size,
                                                 tab_file_element.modifiedDate = finfo.modified_time));
        if (do_sync) { db.commit_transaction(); }
        return FileElementId{ .i = file_element_id };
    } else {
        // the location is known, we may have this file element already
        auto const result_file_element =
            db(select(tab_file_element.fileId)
               .from(tab_file_element)
               .where((tab_file_element.contentId == content_id.i) &&
                      (tab_file_element.modifiedDate == finfo.modified_time)));
        if (!result_file_element.empty()) { return FileElementId{ .i = result_file_element.front().fileId }; }
        // first time we've seen this file with this content, create new file element
        int64_t const file_element_id =
            db(insert_into(tab_file_element).set(tab_file_element.locationId = result_location.front().locationId,
                                                 tab_file_element.contentId = content_id.i,
                                                 tab_file_element.fileSize = finfo.size,
                                                 tab_file_element.modifiedDate = finfo.modified_time));
        return FileElementId{ .i = file_element_id };
    }
}

void BlimpDB::startExternalSync()
{
    m_pimpl->db.execute("PRAGMA synchronous = OFF");
    m_pimpl->db.start_transaction();
}

void BlimpDB::commitExternalSync()
{
    m_pimpl->db.commit_transaction();
    m_pimpl->db.execute("PRAGMA synchronous = FULL");
}
