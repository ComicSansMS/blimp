#ifndef BLIMP_INCLUDE_GUARD_PLUGIN_COMPRESSION_HPP
#define BLIMP_INCLUDE_GUARD_PLUGIN_COMPRESSION_HPP

#include <blimp_plugin_sdk.h>

#include <boost/dll/shared_library.hpp>

#include <memory>

class PluginCompression {
private:
    boost::dll::shared_library m_compression_dll;
    blimp_plugin_compression_initialize_type m_compression_plugin_initialize;
    blimp_plugin_compression_shutdown_type m_compression_plugin_shutdown;
    BlimpPluginCompression* m_compression;
    std::unique_ptr<BlimpPluginCompression, blimp_plugin_compression_shutdown_type> m_compression_guard;
public:
    PluginCompression(std::string const& plugin_name);

    void compressFileChunk(BlimpFileChunk chunk);
    BlimpFileChunk getCompressedChunk();
};

#endif
