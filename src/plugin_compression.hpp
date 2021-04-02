#ifndef BLIMP_INCLUDE_GUARD_PLUGIN_COMPRESSION_HPP
#define BLIMP_INCLUDE_GUARD_PLUGIN_COMPRESSION_HPP

#include <blimp_plugin_sdk.h>

#include <boost/dll/shared_library.hpp>

#include <memory>

class BlimpDB;
class PluginKeyValueStore;

class PluginCompression {
private:
    boost::dll::shared_library m_compression_dll;
    blimp_plugin_api_info_type m_compression_plugin_api_info;
    blimp_plugin_compression_initialize_type m_compression_plugin_initialize;
    blimp_plugin_compression_shutdown_type m_compression_plugin_shutdown;
    BlimpPluginCompression m_compression;
    std::unique_ptr<BlimpPluginCompression, blimp_plugin_compression_shutdown_type> m_compression_guard;
    std::unique_ptr<PluginKeyValueStore> m_kvStore;
public:
    PluginCompression(BlimpDB& blimpdb, std::string const& plugin_name);
    ~PluginCompression();

    BlimpPluginInfo pluginInfo() const;

    char const* getLastError();
    void compressFileChunk(BlimpFileChunk chunk);
    void decompressFileChunk(BlimpFileChunk chunk);
    BlimpFileChunk getProcessedChunk();
};

#endif
