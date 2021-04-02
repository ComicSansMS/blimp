#include <compression_zlib.hpp>

#include <blimp_plugin_helper_cpp.hpp>

#define ZLIB_CONST
#include <zlib.h>

#include <deque>
#include <limits>
#include <stdexcept>
#include <vector>

namespace {
struct ErrorStrings {
    static constexpr char const okay[] = "Ok";
    static constexpr char const compression_error[] = "Unexpected error during compression";
};

struct Buffer {
    std::vector<Bytef> b;
    static constexpr size_t buffer_size = (1 << 20);
    static_assert(buffer_size < std::numeric_limits<uInt>::max());

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
}   // anonymous namespace

struct BlimpPluginCompressionState {
    z_stream zs_compress;
    z_stream zs_decompress;
    Buffer compression_buffer;
    Buffer decompression_buffer;
    std::deque<Buffer> available_buffers;
    Buffer public_buffer;
    std::vector<Buffer> free_buffers;
    char const* error_string;
    KeyValueStore kv_store;
    bool decompression_is_finished;

    BlimpPluginCompressionState(BlimpKeyValueStore const& n_kv_store);
    ~BlimpPluginCompressionState();

    BlimpPluginCompressionState(BlimpPluginCompressionState const&) = delete;
    BlimpPluginCompressionState& operator=(BlimpPluginCompressionState const&) = delete;

    char const* get_last_error();

    BlimpPluginResult compress_file_chunk(BlimpFileChunk chunk);
    BlimpPluginResult decompress_file_chunk(BlimpFileChunk chunk);
    BlimpFileChunk get_processed_chunk();

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
        .uuid = {
            // {8D475098-7DFF-4181-B266-D185542E402C}
            0x8d475098, 0x7dff, 0x4181, { 0xb2, 0x66, 0xd1, 0x85, 0x54, 0x2e, 0x40, 0x2c }
        },
        .name = "zlib Compression",
        .description = "Compression with zlib deflate"
    };
}

char const* blimp_plugin_get_last_error(BlimpPluginCompressionStateHandle state)
{
    return state->get_last_error();
}

BlimpPluginResult blimp_plugin_compress_file_chunk(BlimpPluginCompressionStateHandle state, BlimpFileChunk chunk)
{
    return state->compress_file_chunk(chunk);
}

BlimpPluginResult blimp_plugin_decompress_file_chunk(BlimpPluginCompressionStateHandle state, BlimpFileChunk chunk)
{
    return state->decompress_file_chunk(chunk);
}

BlimpFileChunk blimp_plugin_get_processed_chunk(BlimpPluginCompressionStateHandle state)
{
    return state->get_processed_chunk();
}

BlimpPluginResult blimp_plugin_compression_initialize(BlimpKeyValueStore kv_store, BlimpPluginCompression* plugin)
{
    if (plugin->abi != BLIMP_PLUGIN_ABI_1_0_0) {
        return BLIMP_PLUGIN_RESULT_INVALID_ARGUMENT;
    }
    try {
        plugin->state = new BlimpPluginCompressionState(kv_store);
    } catch (std::exception&) {
        plugin->state = nullptr;
        return BLIMP_PLUGIN_RESULT_FAILED;
    }
    plugin->get_last_error = blimp_plugin_get_last_error;
    plugin->compress_file_chunk = blimp_plugin_compress_file_chunk;
    plugin->decompress_file_chunk = blimp_plugin_decompress_file_chunk;
    plugin->get_processed_chunk = blimp_plugin_get_processed_chunk;
    return BLIMP_PLUGIN_RESULT_OK;
}

void blimp_plugin_compression_shutdown(BlimpPluginCompression* plugin)
{
    delete plugin->state;
}

BlimpPluginCompressionState::BlimpPluginCompressionState(BlimpKeyValueStore const& n_kv_store)
    :kv_store(n_kv_store), decompression_is_finished(false)
{
    error_string = ErrorStrings::okay;
    zs_compress.opaque = nullptr;
    zs_compress.zalloc = nullptr;
    zs_compress.zfree = nullptr;
    int res = deflateInit(&zs_compress, Z_BEST_COMPRESSION);
    if (res != Z_OK) {
        throw std::exception();
    }
    zs_compress.next_out = compression_buffer.data_byte();
    zs_compress.avail_out = static_cast<uInt>(compression_buffer.size());
    zs_compress.next_in = nullptr;
    zs_compress.avail_in = 0;

    zs_decompress.opaque = nullptr;
    zs_decompress.zalloc = nullptr;
    zs_decompress.zfree = nullptr;
    zs_decompress.next_in = nullptr;
    zs_decompress.avail_in = 0;
    res = inflateInit(&zs_decompress);
    if (res != Z_OK) {
        throw std::exception();
    }
    zs_decompress.next_out = decompression_buffer.data_byte();
    zs_decompress.avail_out = static_cast<uInt>(decompression_buffer.size());
}

BlimpPluginCompressionState::~BlimpPluginCompressionState()
{
    deflateEnd(&zs_compress);
    inflateEnd(&zs_decompress);
}

char const* BlimpPluginCompressionState::get_last_error()
{
    return error_string;
}

BlimpPluginResult BlimpPluginCompressionState::compress_file_chunk(BlimpFileChunk chunk)
{
    if (chunk.size > std::numeric_limits<uInt>::max()) {
        return BLIMP_PLUGIN_RESULT_INVALID_ARGUMENT;
    }
    if (chunk.data != nullptr) {
        zs_compress.next_in = reinterpret_cast<Bytef const*>(chunk.data);
        zs_compress.avail_in = static_cast<uInt>(chunk.size);
        while (zs_compress.avail_in > 0) {
            int const res = deflate(&zs_compress, Z_NO_FLUSH);
            if (res != Z_OK) { error_string = ErrorStrings::compression_error; return BLIMP_PLUGIN_RESULT_FAILED; }
            if (zs_compress.avail_out == 0) {
                available_buffers.emplace_back(std::move(compression_buffer));
                compression_buffer = getFreeBuffer();
                zs_compress.next_out = compression_buffer.data_byte();
                zs_compress.avail_out = static_cast<uInt>(compression_buffer.size());
            }
        }
    } else {
        // finalize and flush
        while (true) {
            int const res = deflate(&zs_compress, Z_FINISH);
            if ((res != Z_OK) && (res != Z_STREAM_END)) { error_string = ErrorStrings::compression_error; return BLIMP_PLUGIN_RESULT_FAILED; }
            available_buffers.emplace_back(std::move(compression_buffer));
            compression_buffer = getFreeBuffer();
            if (res == Z_STREAM_END) { break; }
            zs_compress.next_out = compression_buffer.data_byte();
            zs_compress.avail_out = static_cast<uInt>(compression_buffer.size());
        }
        available_buffers.back().b.resize(Buffer::buffer_size - zs_compress.avail_out);
        int const res = deflateReset(&zs_compress);
        if (res != Z_OK) { error_string = ErrorStrings::compression_error; return BLIMP_PLUGIN_RESULT_FAILED; }
        zs_compress.next_out = compression_buffer.data_byte();
        zs_compress.avail_out = static_cast<uInt>(compression_buffer.size());
    }
    return BLIMP_PLUGIN_RESULT_OK;
}

BlimpPluginResult BlimpPluginCompressionState::decompress_file_chunk(BlimpFileChunk chunk)
{
    if (chunk.size > std::numeric_limits<uInt>::max()) {
        return BLIMP_PLUGIN_RESULT_INVALID_ARGUMENT;
    }
    if (chunk.data != nullptr) {
        if (decompression_is_finished) { return BLIMP_PLUGIN_RESULT_FAILED; }
        zs_decompress.next_in = reinterpret_cast<Bytef const*>(chunk.data);
        zs_decompress.avail_in = static_cast<uInt>(chunk.size);
        while (zs_decompress.avail_in > 0) {
            int const res = inflate(&zs_decompress, Z_NO_FLUSH);
            if (res != Z_OK) {
                if (res == Z_STREAM_END) {
                    decompression_buffer.b.resize(decompression_buffer.b.size() - zs_decompress.avail_out);
                    available_buffers.emplace_back(std::move(decompression_buffer));
                    decompression_buffer = getFreeBuffer();
                    zs_decompress.next_out = nullptr;
                    zs_decompress.avail_out = 0;
                    decompression_is_finished = true;
                    break;
                } else {
                    return BLIMP_PLUGIN_RESULT_FAILED;
                }
            }
            if (zs_decompress.avail_out == 0) {
                available_buffers.emplace_back(std::move(decompression_buffer));
                decompression_buffer = getFreeBuffer();
                zs_decompress.next_out = decompression_buffer.data_byte();
                zs_decompress.avail_out = static_cast<uInt>(decompression_buffer.size());
            }
        }
    } else {
        if (!decompression_is_finished) { return BLIMP_PLUGIN_RESULT_FAILED; }
        int const res = inflateReset(&zs_decompress);
        if (res != Z_OK) { return BLIMP_PLUGIN_RESULT_FAILED; }
        zs_decompress.next_out = decompression_buffer.data_byte();
        zs_decompress.avail_out = static_cast<uInt>(decompression_buffer.size());
        decompression_is_finished = false;
    }
    return BLIMP_PLUGIN_RESULT_OK;
}

BlimpFileChunk BlimpPluginCompressionState::get_processed_chunk()
{
    if (!available_buffers.empty()) {
        free_buffers.emplace_back(std::move(public_buffer));
        public_buffer = std::move(available_buffers.front());
        available_buffers.pop_front();
        BlimpFileChunk ret;
        ret.data = public_buffer.data_char();
        ret.size = public_buffer.size();
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

