#include <encryption_aes.hpp>

#include <blimp_plugin_helper_cpp.hpp>

#include <stdexcept>
#include <vector>
#include <limits>

namespace {
struct ErrorStrings {
    static constexpr char const okay[] = "Ok";
    static constexpr char const encryption_error[] = "Unexpected error during encryption";
};
}   // anonymous namespace

struct BlimpPluginEncryptionState {
    char const* error_string;
    KeyValueStore m_kvStore;

    BlimpPluginEncryptionState(BlimpKeyValueStore const& kv_store);
    ~BlimpPluginEncryptionState();

    BlimpPluginEncryptionState(BlimpPluginEncryptionState const&) = delete;
    BlimpPluginEncryptionState& operator=(BlimpPluginEncryptionState const&) = delete;

    char const* get_last_error();
};

BlimpPluginInfo blimp_plugin_api_info()
{
    return BlimpPluginInfo{
        .type = BLIMP_PLUGIN_TYPE_ENCRYPTION,
        .version = BlimpPluginVersion{
            .major = 1,
            .minor = 0,
            .patch = 0
    },
        .uuid = {
            // {2CCA3B08-97F3-4710-B8CA-1E99B8C56AA7}
            0x2cca3b08, 0x97f3, 0x4710, { 0xb8, 0xca, 0x1e, 0x99, 0xb8, 0xc5, 0x6a, 0xa7 }
        },
        .name = "AES Encryption",
        .description = "AES Encryption with Crypto++"
    };
}

char const* blimp_plugin_get_last_error(BlimpPluginEncryptionStateHandle state)
{
    return state->get_last_error();
}

BlimpPluginResult blimp_plugin_encryption_initialize(BlimpKeyValueStore kv_store, BlimpPluginEncryption* plugin)
{
    if (plugin->abi != BLIMP_PLUGIN_ABI_1_0_0) {
        return BLIMP_PLUGIN_INVALID_ARGUMENT;
    }
    try {
        plugin->state = new BlimpPluginEncryptionState(kv_store);
    } catch (std::exception&) {
        plugin->state = nullptr;
        return BLIMP_PLUGIN_RESULT_FAILED;
    }
    plugin->get_last_error = blimp_plugin_get_last_error;
    return BLIMP_PLUGIN_RESULT_OK;
}

void blimp_plugin_encryption_shutdown(BlimpPluginEncryption* plugin)
{
    delete plugin->state;
}

BlimpPluginEncryptionState::BlimpPluginEncryptionState(BlimpKeyValueStore const& kv_store)
    :m_kvStore(kv_store)
{
    error_string = ErrorStrings::okay;
}

BlimpPluginEncryptionState::~BlimpPluginEncryptionState()
{
}

char const* BlimpPluginEncryptionState::get_last_error()
{
    return error_string;
}

