#include <db/blimpdb.hpp>
#include <db/table/table_layout.hpp>
#include <db/table/blimp_properties.hpp>
#include <db/table/sqlite_master.hpp>

#include <exceptions.hpp>
#include <version.hpp>

#include <gbBase/Assert.hpp>
#include <gbBase/Log.hpp>

#include <gsl.h>

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
    return true;
}
}

void createBlimpPropertiesTable(sqlpp::sqlite3::connection& db)
{
    auto const prop_tab = blimpdb::BlimpProperties{};
    db.execute(blimpdb::table_layout::blimp_properties());
    db.execute(blimpdb::table_layout::user_selection());
    db.execute(blimpdb::table_layout::indexed_locations());
    db.execute(blimpdb::table_layout::file_element());
    db.execute(blimpdb::table_layout::file_contents());
    db.execute(blimpdb::table_layout::snapshot());
    db.execute(blimpdb::table_layout::snapshot_contents());

    db(sqlpp::insert_into(prop_tab).set(prop_tab.id    = "version",
                                        prop_tab.value = std::to_string(BlimpVersion::version())));
}


/* static */
void BlimpDB::createNewFileDatabase(std::string const& db_filename, std::vector<std::string> const& initialFileSet)
{
    GHULBUS_LOG(Info, "Creating new database at " << db_filename << " from initial set of "
                      << initialFileSet.size() << " element(s).");

    sqlpp::sqlite3::connection_config conf;
    conf.debug = sqlpp11_debug();
    conf.flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;
    conf.path_to_database = db_filename;

    sqlpp::sqlite3::connection db(conf);
    createBlimpPropertiesTable(db);

    GHULBUS_LOG(Info, " Successfully established database at " << db_filename << ".");
}

/* static */
void BlimpDB::openExistingFileDatabase(std::string const& db_filename)
{
    sqlpp::sqlite3::connection_config conf;
    conf.debug = sqlpp11_debug();
    conf.flags = SQLITE_OPEN_READWRITE;
    conf.path_to_database = db_filename;

    sqlpp::sqlite3::connection db(conf);
    auto const master_tab = blimpdb::SqliteMaster{};
    if(db(sqlpp::select(master_tab.name)
        .from(master_tab)
        .where((master_tab.name == "blimp_properties") && (master_tab.type == "table"))).empty())
    {
        GHULBUS_THROW(Exceptions::DatabaseError(), "Database file does not contain a blimp properties table.");
    }

    auto const prop_tab = blimpdb::BlimpProperties{};
    for(auto const& r : db(sqlpp::select(prop_tab.value).from(prop_tab).where(prop_tab.id == "version")))
    {
        int const version = std::stoi(r.value);
        GHULBUS_LOG(Trace, "Database version " << version);
        if(version != BlimpVersion::version())
        {
            GHULBUS_THROW(Exceptions::DatabaseError(), "Unsupported database version " + std::string(r.value) + ".");
        }
    }
}
