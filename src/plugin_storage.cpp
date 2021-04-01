#include <plugin_storage.hpp>

#include <exceptions.hpp>
#include <plugin_common.hpp>
#include <plugin_key_value_store.hpp>

PluginStorage::PluginStorage(BlimpDB& blimpdb, std::string const& plugin_name)
    :m_storage_guard(nullptr, nullptr)
{
    m_storage_dll = load_plugin_by_name(plugin_name);
    m_storage_plugin_api_info =
        m_storage_dll.get<BlimpPluginInfo()>("blimp_plugin_api_info");
    auto const api_info = m_storage_plugin_api_info();
    if (api_info.type != BLIMP_PLUGIN_TYPE_STORAGE) {
        GHULBUS_THROW(Exceptions::PluginError{}
                      << Exception_Info::Records::plugin_name(plugin_name)
                      << Ghulbus::Exception_Info::filename(m_storage_dll.location().string()),
                      "Plugin is not a storage plugin");
    }
    m_storage_plugin_initialize =
        m_storage_dll.get<BlimpPluginResult(BlimpKeyValueStore, BlimpPluginStorage*)>("blimp_plugin_storage_initialize");
    m_storage_plugin_shutdown =
        m_storage_dll.get<void(BlimpPluginStorage*)>("blimp_plugin_storage_shutdown");
    m_storage.abi = BLIMP_PLUGIN_ABI_1_0_0;
    m_kvStore = std::make_unique<PluginKeyValueStore>(blimpdb, api_info);
    BlimpPluginResult const res = m_storage_plugin_initialize(m_kvStore->getPluginKeyValueStore(), &m_storage);
    if (res != BLIMP_PLUGIN_RESULT_OK) {
        GHULBUS_THROW(Exceptions::PluginError{}
                      << Exception_Info::Records::plugin_name(plugin_name)
                      << Ghulbus::Exception_Info::filename(m_storage_dll.location().string()),
                      "Unable to initialize storage plugin");
    }
    m_storage_guard =
        std::unique_ptr<BlimpPluginStorage, blimp_plugin_storage_shutdown_type>(&m_storage, m_storage_plugin_shutdown);
}

PluginStorage::~PluginStorage() = default;

BlimpPluginInfo PluginStorage::pluginInfo() const
{
    return m_storage_plugin_api_info();
}

char const* PluginStorage::getLastError()
{
    return m_storage.get_last_error(m_storage.state);
}

void PluginStorage::setBaseLocation(char const* path)
{
    BlimpPluginResult const res = m_storage.set_base_location(m_storage.state, path);
    if (res != BLIMP_PLUGIN_RESULT_OK) {
        GHULBUS_THROW(Exceptions::PluginError{}
                      << Ghulbus::Exception_Info::filename(m_storage_dll.location().string())
                      << Exception_Info::Records::plugin_error_code(res)
                      << Exception_Info::Records::plugin_error_message(getLastError()),
                      "Error while setting base location for storage");
    }
}

void PluginStorage::newStorageContainer(StorageContainerId const& container_id)
{
    BlimpPluginResult const res = m_storage.new_storage_container(m_storage.state, container_id.i);
    if (res != BLIMP_PLUGIN_RESULT_OK) {
        GHULBUS_THROW(Exceptions::PluginError{}
                      << Ghulbus::Exception_Info::filename(m_storage_dll.location().string())
                      << Exception_Info::Records::plugin_error_code(res)
                      << Exception_Info::Records::plugin_error_message(getLastError()),
                      "Error while switching storage to new container");
    }
}

BlimpStorageContainerLocation PluginStorage::finalizeStorageContainer()
{
    BlimpStorageContainerLocation out_location{};
    BlimpPluginResult const res = m_storage.finalize_storage_container(m_storage.state, &out_location);
    if (res != BLIMP_PLUGIN_RESULT_OK) {
        GHULBUS_THROW(Exceptions::PluginError{}
                      << Ghulbus::Exception_Info::filename(m_storage_dll.location().string())
                      << Exception_Info::Records::plugin_error_code(res)
                      << Exception_Info::Records::plugin_error_message(getLastError()),
                      "Error while finalizing storage container");
    }
    return out_location;
}

void PluginStorage::storeFileChunk(BlimpFileChunk chunk)
{
    BlimpPluginResult const res = m_storage.store_file_chunk(m_storage.state, chunk);
    if (res != BLIMP_PLUGIN_RESULT_OK) {
        GHULBUS_THROW(Exceptions::PluginError{}
                      << Ghulbus::Exception_Info::filename(m_storage_dll.location().string())
                      << Exception_Info::Records::plugin_error_code(res)
                      << Exception_Info::Records::plugin_error_message(getLastError()),
                      "Error while moving file chunk to storage");
    }
}
