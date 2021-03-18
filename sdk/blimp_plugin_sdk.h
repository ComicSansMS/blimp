#ifndef BLIMP_INCLUDE_GUARD_BLIMP_PLUGIN_SDK_H
#define BLIMP_INCLUDE_GUARD_BLIMP_PLUGIN_SDK_H

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum BlimpPluginType_Tag {
    BLIMP_PLUGIN_TYPE_STORAGE
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

#ifdef __cplusplus
}
#endif
#endif
