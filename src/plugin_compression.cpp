#include <plugin_compression.hpp>

#include <plugin_common.hpp>

PluginCompression::PluginCompression(std::string const& plugin_name)
    :m_compression_guard(nullptr, nullptr)
{
    m_compression_dll = load_plugin_by_name(plugin_name);
    m_compression_plugin_initialize =
        m_compression_dll.get<BlimpPluginCompression* ()>("blimp_plugin_compression_initialize");
    m_compression_plugin_shutdown =
        m_compression_dll.get<void(BlimpPluginCompression*)>("blimp_plugin_compression_shutdown");
    m_compression = m_compression_plugin_initialize();
    m_compression_guard = 
        std::unique_ptr<BlimpPluginCompression, blimp_plugin_compression_shutdown_type>(m_compression,
                                                                                        m_compression_plugin_shutdown);
}

void PluginCompression::compressFileChunk(BlimpFileChunk chunk)
{
    m_compression->compress_file_chunk(m_compression->state, chunk);
}

BlimpFileChunk PluginCompression::getCompressedChunk()
{
    return m_compression->get_compressed_chunk(m_compression->state);
}
