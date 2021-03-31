#ifndef BLIMP_INCLUDE_GUARD_PLUGIN_STORAGE_FILESYSTEM_HPP
#define BLIMP_INCLUDE_GUARD_PLUGIN_STORAGE_FILESYSTEM_HPP

#include <blimp_plugin_sdk.h>

#include <storage_filesystem_export.h>

extern "C" STORAGE_FILESYSTEM_EXPORT BlimpPluginInfo blimp_plugin_api_info();

extern "C" STORAGE_FILESYSTEM_EXPORT BlimpPluginResult blimp_plugin_storage_initialize(BlimpKeyValueStore kv_store,
                                                                                       BlimpPluginStorage* plugin);

extern "C" STORAGE_FILESYSTEM_EXPORT void blimp_plugin_storage_shutdown(BlimpPluginStorage* plugin);

#endif
