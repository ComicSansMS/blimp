#include <storage_filesystem.hpp>

#include <blimp_plugin_helper_cpp.hpp>

#include <boost/filesystem.hpp>
#include <boost/system/error_code.hpp>

#include <algorithm>
#include <array>
#include <deque>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
struct ErrorStrings {
    static constexpr char const okay[] = "Ok";
};
}   // anonymous namespace

struct BlimpPluginStorageState {
    char const* error_string;
    KeyValueStore kv_store;

    boost::filesystem::path m_basePath;
    boost::filesystem::path m_containerPath;
    std::string m_currentLocationString;

    std::ofstream m_fout;

    BlimpPluginStorageState(BlimpKeyValueStore const& n_kv_store);
    ~BlimpPluginStorageState();

    BlimpPluginStorageState(BlimpPluginStorageState const&) = delete;
    BlimpPluginStorageState& operator=(BlimpPluginStorageState const&) = delete;

    char const* get_last_error();
    BlimpPluginResult set_base_location(char const* path);
    BlimpPluginResult new_storage_container(int64_t container_id);
    BlimpPluginResult finalize_storage_container(BlimpStorageContainerLocation* out_location);
    BlimpPluginResult store_file_chunk(BlimpFileChunk const& chunk);
};

BlimpPluginInfo blimp_plugin_api_info()
{
    return BlimpPluginInfo{
        .type = BLIMP_PLUGIN_TYPE_STORAGE,
        .version = BlimpPluginVersion{
            .major = 1,
            .minor = 0,
            .patch = 0
    },
        .uuid = {
            // {0EE5A68C-3720-41B2-989A-CA3903B596B9}
            0xee5a68c, 0x3720, 0x41b2, { 0x98, 0x9a, 0xca, 0x39, 0x3, 0xb5, 0x96, 0xb9 }
    },
        .name = "Blimp File System Storage",
        .description = "Stores backups in a local or remote file system"
    };
}

char const* blimp_plugin_get_last_error(BlimpPluginStorageStateHandle state)
{
    return state->get_last_error();
}

BlimpPluginResult blimp_plugin_set_base_location(BlimpPluginStorageStateHandle state, char const* path)
{
    return state->set_base_location(path);
}

BlimpPluginResult blimp_plugin_new_storage_container(BlimpPluginStorageStateHandle state, int64_t container_id)
{
    return state->new_storage_container(container_id);
}

BlimpPluginResult blimp_plugin_finalize_storage_container(BlimpPluginStorageStateHandle state,
                                                          BlimpStorageContainerLocation* out_location)
{
    return state->finalize_storage_container(out_location);
}

BlimpPluginResult blimp_plugin_store_file_chunk(BlimpPluginStorageStateHandle state, BlimpFileChunk chunk)
{
    return state->store_file_chunk(chunk);
}

BlimpPluginResult blimp_plugin_storage_initialize(BlimpKeyValueStore kv_store, BlimpPluginStorage* plugin)
{
    if (plugin->abi != BLIMP_PLUGIN_ABI_1_0_0) {
        return BLIMP_PLUGIN_RESULT_INVALID_ARGUMENT;
    }
    try {
        plugin->state = new BlimpPluginStorageState(kv_store);
    } catch (std::exception&) {
        plugin->state = nullptr;
        return BLIMP_PLUGIN_RESULT_FAILED;
    }
    plugin->get_last_error = blimp_plugin_get_last_error;
    plugin->set_base_location = blimp_plugin_set_base_location;
    plugin->new_storage_container = blimp_plugin_new_storage_container;
    plugin->finalize_storage_container = blimp_plugin_finalize_storage_container;
    plugin->store_file_chunk = blimp_plugin_store_file_chunk;
    return BLIMP_PLUGIN_RESULT_OK;
}

void blimp_plugin_storage_shutdown(BlimpPluginStorage* plugin)
{
    delete plugin->state;
}


BlimpPluginStorageState::BlimpPluginStorageState(BlimpKeyValueStore const& n_kv_store)
    :error_string(ErrorStrings::okay), kv_store(n_kv_store)
{}

BlimpPluginStorageState::~BlimpPluginStorageState()
{}

char const* BlimpPluginStorageState::get_last_error()
{
    return error_string;
}

BlimpPluginResult BlimpPluginStorageState::set_base_location(char const* path)
{
    boost::filesystem::path const new_base = path;
    boost::system::error_code ec;
    if (!boost::filesystem::exists(new_base, ec)) {
        boost::filesystem::create_directories(new_base, ec);
        if (ec) { return BLIMP_PLUGIN_RESULT_FAILED; }
    } else if(!boost::filesystem::is_directory(new_base, ec)) {
        return BLIMP_PLUGIN_RESULT_FAILED;
    }
    if (ec) { return BLIMP_PLUGIN_RESULT_FAILED; }
    m_basePath = new_base;
    return BLIMP_PLUGIN_RESULT_OK;
}

BlimpPluginResult BlimpPluginStorageState::new_storage_container(int64_t container_id)
{
    boost::filesystem::path const container_sub_dir = std::to_string(container_id / 100);
    boost::filesystem::path const container_file = (((container_id % 100) < 10) ? "0" : "" ) + std::to_string(container_id % 100);
    boost::filesystem::path container_path = m_basePath / container_sub_dir;
    boost::system::error_code ec;
    if (!boost::filesystem::exists(container_path, ec)) {
        boost::filesystem::create_directory(container_path, ec);
        if (ec) { return BLIMP_PLUGIN_RESULT_FAILED; }
    } else if (!boost::filesystem::is_directory(container_path, ec)) {
        return BLIMP_PLUGIN_RESULT_FAILED;
    }
    if (ec) { return BLIMP_PLUGIN_RESULT_FAILED; }
    container_path /= container_file;

    if (boost::filesystem::exists(container_path, ec)) {
        if (ec) { return BLIMP_PLUGIN_RESULT_FAILED; }
        return BLIMP_PLUGIN_RESULT_FAILED;
    }

    m_fout.open(container_path.string(), std::ios_base::binary);
    if (!m_fout) {
        return BLIMP_PLUGIN_RESULT_FAILED;
    }
    m_currentLocationString = container_path.string();

    return BLIMP_PLUGIN_RESULT_OK;
}

BlimpPluginResult BlimpPluginStorageState::finalize_storage_container(BlimpStorageContainerLocation* out_location)
{
    m_fout.close();
    out_location->location = m_currentLocationString.c_str();
    if (!m_fout) { return BLIMP_PLUGIN_RESULT_FAILED; }
    return BLIMP_PLUGIN_RESULT_OK;
}

BlimpPluginResult BlimpPluginStorageState::store_file_chunk(BlimpFileChunk const& chunk)
{
    m_fout.write(chunk.data, chunk.size);
    if (!m_fout) { return BLIMP_PLUGIN_RESULT_FAILED; }
    return BLIMP_PLUGIN_RESULT_OK;
}
