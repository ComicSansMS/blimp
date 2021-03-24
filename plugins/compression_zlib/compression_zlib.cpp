#include <compression_zlib.hpp>

#define ZLIB_CONST
#include <zlib.h>

#include <vector>
#include <limits>

struct Buffer {
    std::vector<Bytef> b;
    static constexpr size_t buffer_size = (1 << 20);

    Buffer()
    {
        b.resize(buffer_size);
    }

    Buffer(Buffer const&) = delete;
    Buffer& operator=(Buffer const&) = delete;

    Buffer(Buffer&&) = default;
    Buffer& operator=(Buffer&&) = default;


    Bytef* data_byte() {
        return b.data();
    }

    char const* data_char() const {
        return reinterpret_cast<char const*>(b.data());
    }

    size_t size() {
        return b.size();
    }
};

struct BlimpPluginCompressionState {
    z_stream zs;
    Buffer buffer;
    std::vector<Buffer> available_buffers;
    std::vector<Buffer> free_buffers;
    BlimpFileChunk ret_chunk;

    BlimpPluginCompressionState();
    ~BlimpPluginCompressionState();

    BlimpPluginCompressionState(BlimpPluginCompressionState const&) = delete;
    BlimpPluginCompressionState& operator=(BlimpPluginCompressionState const&) = delete;

    void compress_file_chunk(BlimpFileChunk chunk);
    BlimpFileChunk get_compressed_chunk();

    Buffer getFreeBuffer();
};

BlimpPluginInfo blimp_plugin_api_info()
{
    return BlimpPluginInfo{
        .type = BLIMP_PLUGIN_TYPE_COMPRESSION,
        .version = BlimpPluginVersion{
            .major = 1,
            .minor = 0,
            .patch = 0
        },
        .name = "zlib Compression",
        .description = "Compression with zlib deflate"
    };
}

void blimp_plugin_compress_file_chunk(BlimpPluginCompressionState* state, BlimpFileChunk chunk)
{
    return state->compress_file_chunk(chunk);
}

BlimpFileChunk blimp_plugin_get_compressed_chunk(BlimpPluginCompressionState* state)
{
    return state->get_compressed_chunk();
}

BlimpPluginCompression* blimp_plugin_compression_initialize()
{
    BlimpPluginCompression* ret = new BlimpPluginCompression;
    ret->state = new BlimpPluginCompressionState;
    ret->compress_file_chunk = blimp_plugin_compress_file_chunk;
    ret->get_compressed_chunk = blimp_plugin_get_compressed_chunk;
    return ret;
}

void blimp_plugin_compression_shutdown(BlimpPluginCompression* plugin)
{
    delete plugin->state;
    delete plugin;
}

BlimpPluginCompressionState::BlimpPluginCompressionState()
{
    zs.opaque = nullptr;
    zs.zalloc = nullptr;
    zs.zfree = nullptr;
    int res = deflateInit(&zs, Z_BEST_COMPRESSION);
    if (res != Z_OK) {
        /// @todo
    }
    zs.next_out = buffer.data_byte();
    zs.avail_out = static_cast<uInt>(buffer.size());
}

BlimpPluginCompressionState::~BlimpPluginCompressionState()
{
    deflateEnd(&zs);
}

void BlimpPluginCompressionState::compress_file_chunk(BlimpFileChunk chunk)
{
    if (chunk.size > std::numeric_limits<uInt>::max()) {
        /// @todo
    }
    if (chunk.data != nullptr) {
        if (reinterpret_cast<Bytef const*>(chunk.data) != zs.next_in) {
            zs.next_in = reinterpret_cast<Bytef const*>(chunk.data);
            zs.avail_in = static_cast<uInt>(chunk.size);
        }
        while (zs.avail_in > 0) {
            int res = deflate(&zs, Z_NO_FLUSH);
            if (res != Z_OK) {
                /// @todo
            }
            if (zs.avail_out == 0) {
                available_buffers.emplace_back(std::move(buffer));
                buffer = getFreeBuffer();
                zs.next_out = buffer.data_byte();
                zs.avail_out = static_cast<uInt>(buffer.size());
            }
        }
    } else {
        while (true) {
            int res = deflate(&zs, Z_FINISH);
            if ((res != Z_OK) && (res != Z_STREAM_END)) {
                ///@ todo
            }
            available_buffers.emplace_back(std::move(buffer));
            buffer = getFreeBuffer();
            if (res == Z_STREAM_END) { break; }
            zs.next_out = buffer.data_byte();
            zs.avail_out = static_cast<uInt>(buffer.size());
        }
        available_buffers.back().b.resize(Buffer::buffer_size - zs.avail_out);
        deflateReset(&zs);
        zs.next_out = buffer.data_byte();
        zs.avail_out = static_cast<uInt>(buffer.size());
    }
}

BlimpFileChunk BlimpPluginCompressionState::get_compressed_chunk()
{
    if (!available_buffers.empty()) {
        free_buffers.emplace_back(std::move(available_buffers.back()));
        available_buffers.pop_back();
        BlimpFileChunk ret;
        ret.data = free_buffers.back().data_char();
        ret.size = free_buffers.back().size();
        return ret;
    } else {
        BlimpFileChunk ret;
        ret.data = nullptr;
        ret.size = 0;
        return ret;
    }
}

Buffer BlimpPluginCompressionState::getFreeBuffer()
{
    if (free_buffers.empty()) { return Buffer{}; }
    Buffer ret = std::move(free_buffers.back());
    free_buffers.pop_back();
    ret.b.resize(Buffer::buffer_size);
    return std::move(ret);
}

