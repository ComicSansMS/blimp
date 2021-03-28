#ifndef BLIMP_INCLUDE_GUARD_PLUGIN_ENCRYPTION_HPP
#define BLIMP_INCLUDE_GUARD_PLUGIN_ENCRYPTION_HPP

#include <blimp_plugin_sdk.h>
#include <db/blimpdb.hpp>

#include <boost/dll/shared_library.hpp>

#include <memory>
#include <string>
#include <string_view>

class BlimpDB;
class PluginKeyValueStore;

class PluginEncryption {
private:
    boost::dll::shared_library m_encryption_dll;
    blimp_plugin_api_info_type m_encryption_plugin_api_info;
    blimp_plugin_encryption_initialize_type m_encryption_plugin_initialize;
    blimp_plugin_encryption_shutdown_type m_encryption_plugin_shutdown;
    BlimpPluginEncryption m_encryption;
    std::unique_ptr<BlimpPluginEncryption, blimp_plugin_encryption_shutdown_type> m_encryption_guard;
    std::unique_ptr<PluginKeyValueStore> m_kvStore;
public:
    PluginEncryption(BlimpDB& blimpdb, std::string const& plugin_name);
    ~PluginEncryption();

    BlimpPluginInfo pluginInfo() const;

    void setPassword(std::string_view password);
    void newStorageContainer(BlimpDB::StorageContainerId id);
};

#endif
