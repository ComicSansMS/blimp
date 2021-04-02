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
    BLIMP_PLUGIN_TYPE_ENCRYPTION,
} BlimpPluginType;

typedef struct BlimpPluginVersion_Tag {
    int32_t major;
    int32_t minor;
    int32_t patch;
} BlimpPluginVersion;

typedef struct BlimpUUID_Tag {
    uint32_t d1;
    uint16_t d2;
    uint16_t d3;
    uint8_t d4[8];
} BlimpUUID;

typedef struct BlimpPluginInfo_Tag {
    BlimpPluginType type;
    BlimpPluginVersion version;
    BlimpUUID uuid;
    char const* name;
    char const* description;
} BlimpPluginInfo;

typedef BlimpPluginInfo(*blimp_plugin_api_info_type)();

typedef enum BlimpPluginResult_Tag {
    BLIMP_PLUGIN_RESULT_OK = 0,
    BLIMP_PLUGIN_RESULT_FAILED,
    BLIMP_PLUGIN_RESULT_INVALID_ARGUMENT,
    BLIMP_PLUGIN_RESULT_CORRUPTED_DATA,
} BlimpPluginResult;

typedef enum BlimpConfigurationType_Tag {
    BLIMP_CONFIGURATION_TYPE_TEXT,
    BLIMP_CONFIGURATION_TYPE_TEXT_PASSWORD,
    BLIMP_CONFIGURATION_TYPE_OPTION,
    BLIMP_CONFIGURATION_TYPE_FILE_PATH,
    BLIMP_CONFIGURATION_TYPE_DIRECTORY_PATH,
} BlimpConfigurationType;

typedef struct BlimpStorageContainerLocation_Tag {
    char const* location;
} BlimpStorageContainerLocation;

typedef struct BlimpFileChunk_Tag {
    char const* data;
    int64_t size;
} BlimpFileChunk;

typedef struct BlimpKeyValueStoreValue_Tag {
    char const* data;
    int64_t size;
} BlimpKeyValueStoreValue;

struct BlimpKeyValueStoreState;
typedef struct BlimpKeyValueStoreState* BlimpKeyValueStoreStateHandle;

typedef struct BlimpKeyValueStore_Tag {
    BlimpKeyValueStoreStateHandle state;
    void (*store)(BlimpKeyValueStoreStateHandle state, char const* key, BlimpKeyValueStoreValue value);
    BlimpKeyValueStoreValue (*retrieve)(BlimpKeyValueStoreStateHandle state, char const* key);
} BlimpKeyValueStore;

struct BlimpPluginCompressionState;
typedef struct BlimpPluginCompressionState* BlimpPluginCompressionStateHandle;

typedef struct BlimpPluginCompression_Tag {
    BlimpPluginABI abi;
    BlimpPluginCompressionStateHandle state;
    char const* (*get_last_error)(BlimpPluginCompressionStateHandle state);
    BlimpPluginResult (*compress_file_chunk)(BlimpPluginCompressionStateHandle state, BlimpFileChunk chunk);
    BlimpPluginResult (*decompress_file_chunk)(BlimpPluginCompressionStateHandle state, BlimpFileChunk chunk);
    BlimpFileChunk (*get_processed_chunk)(BlimpPluginCompressionStateHandle state);
} BlimpPluginCompression;

typedef BlimpPluginResult (*blimp_plugin_compression_initialize_type)(BlimpKeyValueStore, BlimpPluginCompression*);
typedef void (*blimp_plugin_compression_shutdown_type)(BlimpPluginCompression* plugin);


struct BlimpPluginEncryptionState;
typedef struct BlimpPluginEncryptionState* BlimpPluginEncryptionStateHandle;

typedef struct BlimpPluginEncryptionPassword_Tag {
    char const* data;
    int64_t size;
} BlimpPluginEncryptionPassword;

typedef struct BlimpPluginEncryptionKey_Tag {
    char const* data;
    int64_t size;
} BlimpPluginEncryptionKey;

typedef struct BlimpPluginEncryption_Tag {
    BlimpPluginABI abi;
    BlimpPluginEncryptionStateHandle state;
    char const* (*get_last_error)(BlimpPluginEncryptionStateHandle state);
    BlimpPluginResult (*set_password)(BlimpPluginEncryptionStateHandle state, BlimpPluginEncryptionPassword password);
    BlimpPluginResult (*new_storage_container)(BlimpPluginEncryptionStateHandle state, int64_t container_id);
    BlimpPluginResult (*encrypt_file_chunk)(BlimpPluginEncryptionStateHandle state, BlimpFileChunk chunk);
    BlimpPluginResult (*decrypt_file_chunk)(BlimpPluginEncryptionStateHandle state, BlimpFileChunk file_chunk);
    BlimpFileChunk (*get_processed_chunk)(BlimpPluginEncryptionStateHandle state);
} BlimpPluginEncryption;

typedef BlimpPluginResult (*blimp_plugin_encryption_initialize_type)(BlimpKeyValueStore, BlimpPluginEncryption*);
typedef void (*blimp_plugin_encryption_shutdown_type)(BlimpPluginEncryption* plugin);

struct BlimpPluginStorageState;
typedef struct BlimpPluginStorageState* BlimpPluginStorageStateHandle;

typedef struct BlimpPluginStorage_Tag {
    BlimpPluginABI abi;
    BlimpPluginStorageStateHandle state;
    char const* (*get_last_error)(BlimpPluginStorageStateHandle state);
    BlimpPluginResult (*set_base_location)(BlimpPluginStorageStateHandle state, char const* path);
    BlimpPluginResult (*new_storage_container)(BlimpPluginStorageStateHandle state, int64_t container_id);
    BlimpPluginResult (*finalize_storage_container)(BlimpPluginStorageStateHandle state,
                                                    BlimpStorageContainerLocation* out_location);
    BlimpPluginResult (*store_file_chunk)(BlimpPluginStorageStateHandle state, BlimpFileChunk chunk);
} BlimpPluginStorage;

typedef BlimpPluginResult (*blimp_plugin_storage_initialize_type)(BlimpKeyValueStore, BlimpPluginStorage*);
typedef void (*blimp_plugin_storage_shutdown_type)(BlimpPluginStorage* plugin);

#ifdef __cplusplus
}
#endif
#endif
