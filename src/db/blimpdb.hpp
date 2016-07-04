#ifndef BLIMP_INCLUDE_GUARD_DB_BLIMPDB_HPP
#define BLIMP_INCLUDE_GUARD_DB_BLIMPDB_HPP

#include <file_hash.hpp>
#include <file_info.hpp>

#include <memory>
#include <string>
#include <vector>

class BlimpDB
{
public:
    enum class OpenMode {
        OpenExisting,
        CreateNew
    };

    using FileElementId = std::uint64_t;

    struct FileIndexInfo {
        std::uint64_t id;
        FileSyncStatus status;

        FileIndexInfo(std::uint64_t n_id, FileSyncStatus n_status) : id(n_id), status(n_status) {}
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

    std::vector<FileIndexInfo> updateFileIndex(std::vector<FileInfo> const& fresh_index);

    void updateFileContents(std::vector<FileIndexInfo> const& index_info, std::vector<Hash> const& hashes);
private:
    void createNewFileDatabase(std::string const& db_filename);
    void openExistingFileDatabase(std::string const& db_filename);
};

#endif
