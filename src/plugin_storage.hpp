#ifndef BLIMP_INCLUDE_GUARD_PLUGIN_STORAGE_HPP
#define BLIMP_INCLUDE_GUARD_PLUGIN_STORAGE_HPP

#include <blimp_plugin_sdk.h>
#include <db/blimpdb.hpp>

#include <boost/dll/shared_library.hpp>

#include <memory>
#include <string>
#include <string_view>

class BlimpDB;
class PluginKeyValueStore;

class PluginStorage {
private:
    boost::dll::shared_library m_storage_dll;
    blimp_plugin_api_info_type m_storage_plugin_api_info;
    blimp_plugin_storage_initialize_type m_storage_plugin_initialize;
    blimp_plugin_storage_shutdown_type m_storage_plugin_shutdown;
    BlimpPluginStorage m_storage;
    std::unique_ptr<BlimpPluginStorage, blimp_plugin_storage_shutdown_type> m_storage_guard;
    std::unique_ptr<PluginKeyValueStore> m_kvStore;
public:
    PluginStorage(BlimpDB& blimpdb, std::string const& plugin_name);
    ~PluginStorage();

    BlimpPluginInfo pluginInfo() const;

    char const* getLastError();
    void setBaseLocation(char const* path);
    void newStorageContainer(StorageContainerId const& container_id);
    BlimpStorageContainerLocation finalizeStorageContainer();
    void storeFileChunk(BlimpFileChunk chunk);
};

#endif
