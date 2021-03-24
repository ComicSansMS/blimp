#ifndef BLIMP_INCLUDE_GUARD_DB_BLIMPDB_HPP
#define BLIMP_INCLUDE_GUARD_DB_BLIMPDB_HPP

#include <file_info.hpp>

#include <date/date.h>

#include <memory>
#include <optional>
#include <string>
#include <span>
#include <tuple>
#include <vector>

struct Hash;
struct StorageLocation;

class BlimpDB
{
public:
    enum class OpenMode {
        OpenExisting,
        CreateNew
    };

    struct FileIndexInfo {
        std::uint64_t id;
        FileSyncStatus status;

        FileIndexInfo(std::uint64_t n_id, FileSyncStatus n_status) : id(n_id), status(n_status) {}
    };

    struct SnapshotId {
        int64_t i;
    };

    struct FileElementId {
        int64_t i;
    };

    struct FileContentId {
        int64_t i;
    };

    struct SnapshotInfo {
        SnapshotId id;
        std::string name;
        date::year_month_day date;
    };

    enum class FileContentInsertion {
        CreatedNew,
        ReferencedExisting
    };
private:
    struct Pimpl;
    std::unique_ptr<Pimpl> m_pimpl;
public:
    BlimpDB(std::string const& db_filename, OpenMode mode);
    ~BlimpDB();
    BlimpDB(BlimpDB const&) = delete;
    BlimpDB& operator=(BlimpDB const&) = delete;

    void setUserSelection(std::vector<std::string> const& selected_files);

    std::vector<std::string> getUserSelection();

    FileIndexDiff compareFileIndex(std::vector<FileInfo> const& fresh_index);

    std::vector<FileIndexInfo> updateFileIndex(std::vector<FileInfo> const& fresh_index,
                                               std::vector<Hash> const& hashes);

    std::vector<SnapshotInfo> getSnapshots();

    SnapshotId addSnapshot(std::string const& name);

    std::tuple<FileElementId, FileContentId, FileContentInsertion>
        newFileContent(FileInfo const& finfo, Hash const& hash, bool do_sync = true);

    FileElementId newFileElement(FileInfo const& finfo, FileContentId const& content_id, bool do_sync = true);

    void newStorageElement(FileContentId const& content_id,
                           std::span<StorageLocation const> const& storage_locations,
                           bool do_sync = true);

    void addSnapshotContents(SnapshotId const& snapshot_id,
                             std::span<FileElementId const> const& files,
                             bool do_sync = true);

    void startExternalSync();
    void commitExternalSync();
private:
    void createNewFileDatabase(std::string const& db_filename);
    void openExistingFileDatabase(std::string const& db_filename);
};

#endif
