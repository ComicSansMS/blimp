#include <plugin_compression.hpp>

#include <exceptions.hpp>
#include <plugin_common.hpp>
#include <plugin_key_value_store.hpp>

PluginCompression::PluginCompression(BlimpDB& blimpdb, std::string const& plugin_name)
    :m_compression_guard(nullptr, nullptr)
{
    m_compression_dll = load_plugin_by_name(plugin_name);
    m_compression_plugin_api_info =
        m_compression_dll.get<BlimpPluginInfo()>("blimp_plugin_api_info");
    auto const api_info = m_compression_plugin_api_info();
    if (api_info.type != BLIMP_PLUGIN_TYPE_COMPRESSION) {
        GHULBUS_THROW(Exceptions::PluginError{}
                      << Exception_Info::Records::plugin_name(plugin_name)
                      << Ghulbus::Exception_Info::filename(m_compression_dll.location().string()),
                      "Plugin is not a compression plugin");
    }
    m_compression_plugin_initialize =
        m_compression_dll.get<BlimpPluginResult(BlimpKeyValueStore, BlimpPluginCompression*)>("blimp_plugin_compression_initialize");
    m_compression_plugin_shutdown =
        m_compression_dll.get<void(BlimpPluginCompression*)>("blimp_plugin_compression_shutdown");
    m_compression.abi = BLIMP_PLUGIN_ABI_1_0_0;
    m_kvStore = std::make_unique<PluginKeyValueStore>(blimpdb, api_info);
    BlimpPluginResult const res = m_compression_plugin_initialize(m_kvStore->getPluginKeyValueStore(), &m_compression);
    if (res != BLIMP_PLUGIN_RESULT_OK) {
        GHULBUS_THROW(Exceptions::PluginError{}
                      << Exception_Info::Records::plugin_name(plugin_name)
                      << Ghulbus::Exception_Info::filename(m_compression_dll.location().string()),
                      "Unable to initialize compression plugin");
    }
    m_compression_guard =
        std::unique_ptr<BlimpPluginCompression, blimp_plugin_compression_shutdown_type>(&m_compression,
                                                                                        m_compression_plugin_shutdown);
}

PluginCompression::~PluginCompression() = default;

BlimpPluginInfo PluginCompression::pluginInfo() const
{
    return m_compression_plugin_api_info();
}

char const* PluginCompression::getLastError()
{
    return m_compression.get_last_error(m_compression.state);
}

void PluginCompression::compressFileChunk(BlimpFileChunk chunk)
{
    BlimpPluginResult const res = m_compression.compress_file_chunk(m_compression.state, chunk);
    if (res != BLIMP_PLUGIN_RESULT_OK) {
        GHULBUS_THROW(Exceptions::PluginError{}
                      << Ghulbus::Exception_Info::filename(m_compression_dll.location().string())
                      << Exception_Info::Records::plugin_error_code(res)
                      << Exception_Info::Records::plugin_error_message(this->getLastError()),
                      "Error while adding file chunk for compression");
    }
}

void PluginCompression::decompressFileChunk(BlimpFileChunk chunk)
{
    BlimpPluginResult const res = m_compression.decompress_file_chunk(m_compression.state, chunk);
    if (res != BLIMP_PLUGIN_RESULT_OK) {
        GHULBUS_THROW(Exceptions::PluginError{}
                      << Ghulbus::Exception_Info::filename(m_compression_dll.location().string())
                      << Exception_Info::Records::plugin_error_code(res)
                      << Exception_Info::Records::plugin_error_message(this->getLastError()),
                      "Error while adding file chunk for decompression");
    }
}

BlimpFileChunk PluginCompression::getProcessedChunk()
{
    return m_compression.get_processed_chunk(m_compression.state);
}
