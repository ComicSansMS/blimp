#ifndef BLIMP_INCLUDE_GUARD_PLUGIN_COMPRESSION_ZLIB_HPP
#define BLIMP_INCLUDE_GUARD_PLUGIN_COMPRESSION_ZLIB_HPP

#include <blimp_plugin_sdk.h>

#include <compression_zlib_export.h>

extern "C" COMPRESSION_ZLIB_EXPORT BlimpPluginInfo blimp_plugin_api_info();

extern "C" COMPRESSION_ZLIB_EXPORT BlimpPluginResult blimp_plugin_compression_initialize(BlimpKeyValueStore kv_store,
                                                                                         BlimpPluginCompression* plugin);

extern "C" COMPRESSION_ZLIB_EXPORT void blimp_plugin_compression_shutdown(BlimpPluginCompression* plugin);

#endif
