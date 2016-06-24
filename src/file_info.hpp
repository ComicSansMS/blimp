#ifndef BLIMP_INCLUDE_GUARD_FILE_INFO_HPP
#define BLIMP_INCLUDE_GUARD_FILE_INFO_HPP

#include <boost/filesystem/path.hpp>

#include <chrono>
#include <cstdint>

enum class FileSyncStatus {
    Unchanged,
    NewFile,
    FileChanged,
    FileRemoved
};

struct FileInfo {
    boost::filesystem::path path;
    std::uintmax_t size;
    std::chrono::system_clock::time_point modified_time;
};

#endif
