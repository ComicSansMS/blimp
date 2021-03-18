#include <storage_filesystem.hpp>

int foo()
{
    return 42;
}

BlimpPluginInfo blimp_plugin_api_info()
{
    BlimpPluginInfo ret;
    ret.type = BLIMP_PLUGIN_TYPE_STORAGE;
    ret.version.major = 0;
    ret.version.minor = 1;
    ret.version.patch = 0;
    ret.name = "Blimp Filesystem";
    ret.description = "Plugin for storage in the local filesystem";
    return ret;
}
