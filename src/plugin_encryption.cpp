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

char const* PluginEncryption::getLastError()
{
    return m_encryption.get_last_error(m_encryption.state);
}

void PluginEncryption::setPassword(std::string_view password)
{
    BlimpPluginEncryptionPassword const p{
        .data = password.data(),
        .size = static_cast<int64_t>(password.size())
    };
    BlimpPluginResult const res = m_encryption.set_password(m_encryption.state, p);
    if (res != BLIMP_PLUGIN_RESULT_OK) {
        GHULBUS_THROW(Exceptions::PluginError{}
                      << Ghulbus::Exception_Info::filename(m_encryption_dll.location().string())
                      << Exception_Info::Records::plugin_error_code(res)
                      << Exception_Info::Records::plugin_error_message(getLastError()),
                      "Error while setting encryption password");
    }
}

void PluginEncryption::newStorageContainer(StorageContainerId id)
{
    BlimpPluginResult const res = m_encryption.new_storage_container(m_encryption.state, id.i);
    if (res != BLIMP_PLUGIN_RESULT_OK) {
        GHULBUS_THROW(Exceptions::PluginError{}
                      << Ghulbus::Exception_Info::filename(m_encryption_dll.location().string())
                      << Exception_Info::Records::plugin_error_code(res)
                      << Exception_Info::Records::plugin_error_message(getLastError()),
                      "Error while switching encryption to new container");
    }
}

void PluginEncryption::encryptFileChunk(BlimpFileChunk chunk)
{
    BlimpPluginResult const res = m_encryption.encrypt_file_chunk(m_encryption.state, chunk);
    if (res != BLIMP_PLUGIN_RESULT_OK) {
        GHULBUS_THROW(Exceptions::PluginError{}
                      << Ghulbus::Exception_Info::filename(m_encryption_dll.location().string())
                      << Exception_Info::Records::plugin_error_code(res)
                      << Exception_Info::Records::plugin_error_message(getLastError()),
                      "Error while during data encryption");
    }
}

void PluginEncryption::decryptFileChunk(BlimpFileChunk chunk)
{
    BlimpPluginResult const res = m_encryption.decrypt_file_chunk(m_encryption.state, chunk);
    if (res != BLIMP_PLUGIN_RESULT_OK) {
        GHULBUS_THROW(Exceptions::PluginError{}
                      << Ghulbus::Exception_Info::filename(m_encryption_dll.location().string())
                      << Exception_Info::Records::plugin_error_code(res)
                      << Exception_Info::Records::plugin_error_message(getLastError()),
                      "Error while during data encryption");
    }
}

BlimpFileChunk PluginEncryption::getProcessedChunk()
{
    return m_encryption.get_processed_chunk(m_encryption.state);
}
