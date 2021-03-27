#ifndef BLIMP_INCLUDE_GUARD_PLUGIN_ENCRYPTION_AES_HPP
#define BLIMP_INCLUDE_GUARD_PLUGIN_ENCRYPTION_AES_HPP

#include <blimp_plugin_sdk.h>

#include <encryption_aes_export.h>

extern "C" ENCRYPTION_AES_EXPORT BlimpPluginInfo blimp_plugin_api_info();

extern "C" ENCRYPTION_AES_EXPORT BlimpPluginResult blimp_plugin_encryption_initialize(BlimpKeyValueStore kv_store,
                                                                                      BlimpPluginEncryption* plugin);

extern "C" ENCRYPTION_AES_EXPORT void blimp_plugin_encryption_shutdown(BlimpPluginEncryption* plugin);

#endif
