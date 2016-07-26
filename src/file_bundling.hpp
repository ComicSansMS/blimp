#ifndef BLIMP_INCLUDE_GUARD_FILE_BUNDLING_HPP
#define BLIMP_INCLUDE_GUARD_FILE_BUNDLING_HPP

#include <file_info.hpp>

struct FileBundleInfo {
    std::uint32_t bundle_id;
};

std::vector<FileBundleInfo> bundleFiles(std::vector<FileInfo> const& files, std::uintmax_t min_unbundled_size);

#endif
