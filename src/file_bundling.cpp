#include <file_bundling.hpp>

#include <gbBase/Assert.hpp>

#include <algorithm>
#include <vector>

std::vector<FileBundleInfo> bundleFiles(std::vector<FileInfo> const& files, std::uintmax_t min_unbundled_size)
{
    std::vector<FileBundleInfo> ret;
    ret.reserve(files.size());
    // the algorithm traverses the file list, adding small elements to the current bundle;
    // once the current bundle size exceeds 3*min_unbundled_size, it gets cut into two bundles which both
    // exceed min_unbundled_size; elements with higher indices move to the bundle with the new id, which then
    // becomes the new current bundle.
    // if the total accumulated data is greater min_unbundled_size, no bundle will be smaller than min_unbundled_size.
    std::vector<std::size_t> bundle_elements;
    std::uint32_t bundle_id = 1u;       // running id of the current bundle assigned during list traversal
    std::uintmax_t size_acc = 0;        // accumulated size of all elements in current bundle
    std::uintmax_t cut_point = 0;       // byte offset into the current bundle at which we could split off a bundle
    std::size_t cut_element_index = 0;  // index of the element preceding cut_point
    for(int i = 0; i < files.size(); ++i) {
        auto const& f = files[i];
        if(f.size < min_unbundled_size) {
            FileBundleInfo bi;
            bi.bundle_id = bundle_id;
            ret.push_back(bi);
            bundle_elements.push_back(i);
            size_acc += f.size;
            if((cut_point == 0) && (size_acc > min_unbundled_size)) {
                // place initial cut point;
                // this happens only once in the beginning when we first exceed min_unbundled_size
                cut_point = size_acc;
                cut_element_index = i;
            }
            if(size_acc > 3*min_unbundled_size) {
                // bundle is big enough to allow cutting into two, both bigger than min_unbundled_size
                GHULBUS_ASSERT(cut_point != 0);
                // move cut_point
                size_acc -= cut_point;
                GHULBUS_ASSERT(size_acc > min_unbundled_size);
                cut_point = size_acc;
                // elements below cut point are committed to their bundle
                auto const cut_it = std::upper_bound(begin(bundle_elements), end(bundle_elements), cut_element_index);
                GHULBUS_ASSERT(cut_it != end(bundle_elements));
                bundle_elements.erase(cut_it, end(bundle_elements));
                cut_element_index = i;
                // elements above cut point move to the next bundle
                ++bundle_id;
                for(auto const& element_index : bundle_elements) {
                    ret[element_index].bundle_id = bundle_id;
                }
            }
        } else {
            // file is too big to be bundled
            FileBundleInfo bi;
            bi.bundle_id = 0u;
            ret.push_back(bi);
        }
    }
    return ret;
}

