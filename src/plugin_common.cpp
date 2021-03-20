#include <plugin_common.hpp>

#include <gbBase/Exception.hpp>

#include <boost/filesystem.hpp>
#include <boost/predef.h>

#define TO_STRING_UNWRAPPED(x) #x
#define TO_STRING(x) TO_STRING_UNWRAPPED(x)
#define BLIMP_BUILD_CONFIGURATION_STRING TO_STRING(BLIMP_BUILD_CONFIGURATION)

boost::dll::shared_library load_plugin_by_name(std::string const& plugin_name)
{
    boost::filesystem::path const search_paths[] = {
        boost::filesystem::path("plugins"),
        boost::filesystem::path("plugins") / plugin_name,
        boost::filesystem::path("plugins") / plugin_name / BLIMP_BUILD_CONFIGURATION_STRING
    };
    std::string const file_extension = ".dll";
    boost::filesystem::path plugin_path;
    for (auto const p : search_paths) {
        boost::filesystem::path candidate = p / (plugin_name + file_extension);
        if (boost::filesystem::exists(candidate) && boost::filesystem::is_regular_file(candidate)) {
            plugin_path = candidate;
            break;
        }
    }
    if (plugin_path.empty()) {
        GHULBUS_THROW(Ghulbus::Exceptions::IOError{}, "Plugin not found");
    }
    return boost::dll::shared_library(plugin_path);
}
