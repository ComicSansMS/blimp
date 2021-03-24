#ifndef BLIMP_INCLUDE_GUARD_PLUGIN_STORAGE_FILESYSTEM_HPP
#define BLIMP_INCLUDE_GUARD_PLUGIN_STORAGE_FILESYSTEM_HPP

#include <blimp_plugin_sdk.h>

#include <storage_filesystem_export.h>

extern "C" STORAGE_FILESYSTEM_EXPORT BlimpPluginInfo blimp_plugin_api_info();

void initialize_storage(char const* target_path);

void new_file();

void add_file_data();

void finish_file();

#endif
