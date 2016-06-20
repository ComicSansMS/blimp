#ifndef BLIMP_INCLUDE_GUARD_FILE_INFO_HPP
#define BLIMP_INCLUDE_GUARD_FILE_INFO_HPP

#include <boost/filesystem/path.hpp>

#include <chrono>
#include <cstdint>
#include <vector>

#include <QMetaType>

struct FileInfo {
    boost::filesystem::path path;
    std::uintmax_t size;
    std::chrono::system_clock::time_point modified_time;
    std::string hash_digest;
};

struct FileIndexResult {
    std::vector<FileInfo> filesIndexed;
    std::vector<boost::filesystem::path> filesSkipped;
};

Q_DECLARE_METATYPE(FileIndexResult)
#endif
