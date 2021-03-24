#ifndef BLIMP_INCLUDE_GUARD_FILE_BUNDLING_HPP
#define BLIMP_INCLUDE_GUARD_FILE_BUNDLING_HPP

#include <file_info.hpp>

struct FileBundleInfo {
    std::uint32_t bundle_id;
};

/** Groups elements from the file list into bundles of a minimum size.
 * Grouping is done by assigning each element from the file list a bundle id.
 * Elements that are by themselves already bigger than the minimum size remain unbundled and get a bundle id of 0.
 * Smaller elements share the same bundle id such that the total size of all elements in a bundle exceeds
 * the minimum size, but is never bigger than three times the minimum size.
 * If the total size of all elements smaller than the minimum size does not exceed the minimum size, all of those
 * elements get assigned to the same bundle.
 * @param[in] files The list of files to be bundled.
 * @param[in] min_unbundled_size The minimum size in bytes for a bundle of files.
 * @return The list of bundle ids for the elements in files. Each element in this vector corresponds to the element
 *         in files of the same index. Bundle ids are strictly increasing. After a bundle id of N in the vector, all
 *         following ids will be >=N or 0.
 */
std::vector<FileBundleInfo> bundleFiles(std::vector<FileInfo> const& files, std::uint64_t min_unbundled_size);

#endif
