#ifndef BLIMP_INCLUDE_GUARD_BLIMP_PLUGIN_SDK_H
#define BLIMP_INCLUDE_GUARD_BLIMP_PLUGIN_SDK_H

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum BlimpPluginType_Tag {
    BLIMP_PLUGIN_TYPE_STORAGE,
    BLIMP_PLUGIN_TYPE_COMPRESSION,
    BLIMP_PLUGIN_TYPE_PROCESSING,
} BlimpPluginType;

typedef struct BlimpPluginVersion_Tag {
    int32_t major;
    int32_t minor;
    int32_t patch;
} BlimpPluginVersion;

typedef struct BlimpPluginInfo_Tag {
    BlimpPluginType type;
    BlimpPluginVersion version;
    char const* name;
    char const* description;
} BlimpPluginInfo;

typedef BlimpPluginInfo(*blimp_plugin_api_info_type)();

typedef enum BlimpConfigurationType_Tag {
    BLIMP_CONFIGURATION_TYPE_TEXT,
    BLIMP_CONFIGURATION_TYPE_TEXT_PASSWORD,
    BLIMP_CONFIGURATION_TYPE_OPTION,
    BLIMP_CONFIGURATION_TYPE_FILE_PATH,
    BLIMP_CONFIGURATION_TYPE_DIRECTORY_PATH,
} BlimpConfigurationType;

typedef struct BlimpStorageLocation_Tag {
    char const* location;
    int64_t offset;
    int64_t size;
} BlimpStorageLocation;

typedef struct BlimpFileChunk_Tag {
    char const* data;
    int64_t size;
} BlimpFileChunk;

struct BlimpPluginCompressionState;

typedef struct BlimpPluginCompression_Tag {
    struct BlimpPluginCompressionState* state;
    void (*compress_file_chunk)(BlimpPluginCompressionState* state, BlimpFileChunk chunk);
    BlimpFileChunk (*get_compressed_chunk)(BlimpPluginCompressionState* state);
} BlimpPluginCompression;

typedef BlimpPluginCompression* (*blimp_plugin_compression_initialize_type)();
typedef void (*blimp_plugin_compression_shutdown_type)(BlimpPluginCompression* plugin);

#ifdef __cplusplus
}
#endif
#endif
