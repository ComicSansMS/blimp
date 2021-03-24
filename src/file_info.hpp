#ifndef BLIMP_INCLUDE_GUARD_FILE_INFO_HPP
#define BLIMP_INCLUDE_GUARD_FILE_INFO_HPP

#include <boost/filesystem/path.hpp>

#include <chrono>
#include <cstdint>
#include <vector>

enum class FileSyncStatus {
    Unchanged,
    NewFile,
    FileChanged,
    FileRemoved
};

struct FileInfo {
    boost::filesystem::path path;
    std::uint64_t size;
    std::chrono::system_clock::time_point modified_time;
};

struct FileIndexDiff {
    struct ElementDiff {
        FileSyncStatus sync_status;
        int64_t reference_db_id;
        std::uint64_t reference_size;
        std::chrono::system_clock::time_point reference_modified_time;
    };
    std::vector<ElementDiff> index_files;
    std::vector<FileInfo> removed_files;
};

#endif
