#ifndef BLIMP_INCLUDE_GUARD_BLIMP_PLUGIN_SDK_H
#define BLIMP_INCLUDE_GUARD_BLIMP_PLUGIN_SDK_H

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum BlimpPluginABI_Tag {
    BLIMP_PLUGIN_ABI_1_0_0 = 1
} BlimpPluginABI;

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

typedef struct BlimpGUID_Tag {
    uint32_t d1;
    uint16_t d2;
    uint16_t d3;
    uint8_t d4[8];
} BlimpGUID;

typedef struct BlimpPluginInfo_Tag {
    BlimpPluginType type;
    BlimpPluginVersion version;
    BlimpGUID guid;
    char const* name;
    char const* description;
} BlimpPluginInfo;

typedef BlimpPluginInfo(*blimp_plugin_api_info_type)();

typedef enum BlimpPluginResult_Tag {
    BLIMP_PLUGIN_RESULT_OK = 0,
    BLIMP_PLUGIN_RESULT_FAILED,
    BLIMP_PLUGIN_INVALID_ARGUMENT,
} BlimpPluginResult;

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
    BlimpPluginABI abi;
    struct BlimpPluginCompressionState* state;
    char const* (*get_last_error)(BlimpPluginCompressionState* state);
    BlimpPluginResult (*compress_file_chunk)(BlimpPluginCompressionState* state, BlimpFileChunk chunk);
    BlimpFileChunk (*get_compressed_chunk)(BlimpPluginCompressionState* state);
} BlimpPluginCompression;

typedef char const* (*blimp_plugin_compression_get_last_error_type)(BlimpPluginCompression* plugin);
typedef BlimpPluginResult (*blimp_plugin_compression_initialize_type)(BlimpPluginCompression*);
typedef void (*blimp_plugin_compression_shutdown_type)(BlimpPluginCompression* plugin);

#ifdef __cplusplus
}
#endif
#endif
