#include <plugin_encryption.hpp>

#include <exceptions.hpp>
#include <plugin_common.hpp>
#include <plugin_key_value_store.hpp>

PluginEncryption::PluginEncryption(BlimpDB& blimpdb, std::string const& plugin_name)
    :m_encryption_guard(nullptr, nullptr)
{
    m_encryption_dll = load_plugin_by_name(plugin_name);
    m_encryption_plugin_api_info =
        m_encryption_dll.get<BlimpPluginInfo()>("blimp_plugin_api_info");
    auto const api_info = m_encryption_plugin_api_info();
    if (api_info.type != BLIMP_PLUGIN_TYPE_ENCRYPTION) {
        GHULBUS_THROW(Exceptions::PluginError{}
                      << Exception_Info::Records::plugin_name(plugin_name)
                      << Ghulbus::Exception_Info::filename(m_encryption_dll.location().string()),
                      "Plugin is not an encryption plugin");
    }
    m_encryption_plugin_initialize =
        m_encryption_dll.get<BlimpPluginResult(BlimpKeyValueStore, BlimpPluginEncryption*)>("blimp_plugin_encryption_initialize");
    m_encryption_plugin_shutdown =
        m_encryption_dll.get<void(BlimpPluginEncryption*)>("blimp_plugin_encryption_shutdown");
    m_encryption.abi = BLIMP_PLUGIN_ABI_1_0_0;
    m_kvStore = std::make_unique<PluginKeyValueStore>(blimpdb, api_info);
    BlimpPluginResult const res = m_encryption_plugin_initialize(m_kvStore->getPluginKeyValueStore(), &m_encryption);
    if (res != BLIMP_PLUGIN_RESULT_OK) {
        GHULBUS_THROW(Exceptions::PluginError{}
                      << Exception_Info::Records::plugin_name(plugin_name)
                      << Ghulbus::Exception_Info::filename(m_encryption_dll.location().string()),
                      "Unable to initialize encryption plugin");
    }
    m_encryption_guard =
        std::unique_ptr<BlimpPluginEncryption, blimp_plugin_encryption_shutdown_type>(&m_encryption,
                                                                                      m_encryption_plugin_shutdown);
}

PluginEncryption::~PluginEncryption() = default;

BlimpPluginInfo PluginEncryption::pluginInfo() const
{
    return m_encryption_plugin_api_info();
}
