#include <compression_zlib.hpp>

#define ZLIB_CONST
#include <zlib.h>

#include <stdexcept>
#include <vector>
#include <limits>

namespace {
struct ErrorStrings {
    static constexpr char const okay[] = "Ok";
    static constexpr char const compression_error[] = "Unexpected error during compression";
};

class KeyValueStore {
private:
    BlimpKeyValueStore m_kv;
public:
    explicit KeyValueStore(BlimpKeyValueStore const& kv)
        :m_kv(kv)
    {}

    void store(char const* key, BlimpKeyValueStoreValue v) {
        m_kv.store(m_kv.state, key, v);
    }

    BlimpKeyValueStoreValue retrieve(char const* key) {
        return m_kv.retrieve(m_kv.state, key);
    }
};

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
}   // anonymous namespace

struct BlimpPluginCompressionState {
    z_stream zs;
    Buffer buffer;
    std::vector<Buffer> available_buffers;
    Buffer public_buffer;
    std::vector<Buffer> free_buffers;
    char const* error_string;
    KeyValueStore m_kvStore;

    BlimpPluginCompressionState(BlimpKeyValueStore const& kv_store);
    ~BlimpPluginCompressionState();

    BlimpPluginCompressionState(BlimpPluginCompressionState const&) = delete;
    BlimpPluginCompressionState& operator=(BlimpPluginCompressionState const&) = delete;

    char const* get_last_error();

    BlimpPluginResult compress_file_chunk(BlimpFileChunk chunk);
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

BlimpFileChunk blimp_plugin_get_compressed_chunk(BlimpPluginCompressionStateHandle state)
{
    return state->get_compressed_chunk();
}

BlimpPluginResult blimp_plugin_compression_initialize(BlimpKeyValueStore kv_store, BlimpPluginCompression* plugin)
{
    if (plugin->abi != BLIMP_PLUGIN_ABI_1_0_0) {
        return BLIMP_PLUGIN_INVALID_ARGUMENT;
    }
    try {
        plugin->state = new BlimpPluginCompressionState(kv_store);
    } catch (std::exception&) {
        plugin->state = nullptr;
        return BLIMP_PLUGIN_RESULT_FAILED;
    }
    plugin->get_last_error = blimp_plugin_get_last_error;
    plugin->compress_file_chunk = blimp_plugin_compress_file_chunk;
    plugin->get_compressed_chunk = blimp_plugin_get_compressed_chunk;
    return BLIMP_PLUGIN_RESULT_OK;
}

void blimp_plugin_compression_shutdown(BlimpPluginCompression* plugin)
{
    delete plugin->state;
}

BlimpPluginCompressionState::BlimpPluginCompressionState(BlimpKeyValueStore const& kv_store)
    :m_kvStore(kv_store)
{
    error_string = ErrorStrings::okay;
    zs.opaque = nullptr;
    zs.zalloc = nullptr;
    zs.zfree = nullptr;
    int res = deflateInit(&zs, Z_BEST_COMPRESSION);
    if (res != Z_OK) {
        throw std::exception();
    }
    zs.next_out = buffer.data_byte();
    zs.avail_out = static_cast<uInt>(buffer.size());
}

BlimpPluginCompressionState::~BlimpPluginCompressionState()
{
    deflateEnd(&zs);
}

char const* BlimpPluginCompressionState::get_last_error()
{
    return error_string;
}

BlimpPluginResult BlimpPluginCompressionState::compress_file_chunk(BlimpFileChunk chunk)
{
    if (chunk.size > std::numeric_limits<uInt>::max()) {
        return BLIMP_PLUGIN_INVALID_ARGUMENT;
    }
    if (chunk.data != nullptr) {
        if (reinterpret_cast<Bytef const*>(chunk.data) != zs.next_in) {
            zs.next_in = reinterpret_cast<Bytef const*>(chunk.data);
            zs.avail_in = static_cast<uInt>(chunk.size);
        }
        while (zs.avail_in > 0) {
            int const res = deflate(&zs, Z_NO_FLUSH);
            if (res != Z_OK) { error_string = ErrorStrings::compression_error; return BLIMP_PLUGIN_RESULT_FAILED; }
            if (zs.avail_out == 0) {
                available_buffers.emplace_back(std::move(buffer));
                buffer = getFreeBuffer();
                zs.next_out = buffer.data_byte();
                zs.avail_out = static_cast<uInt>(buffer.size());
            }
        }
    } else {
        // finalize and flush
        while (true) {
            int const res = deflate(&zs, Z_FINISH);
            if ((res != Z_OK) && (res != Z_STREAM_END)) { error_string = ErrorStrings::compression_error; return BLIMP_PLUGIN_RESULT_FAILED; }
            available_buffers.emplace_back(std::move(buffer));
            buffer = getFreeBuffer();
            if (res == Z_STREAM_END) { break; }
            zs.next_out = buffer.data_byte();
            zs.avail_out = static_cast<uInt>(buffer.size());
        }
        available_buffers.back().b.resize(Buffer::buffer_size - zs.avail_out);
        int const res = deflateReset(&zs);
        if (res != Z_OK) { error_string = ErrorStrings::compression_error; return BLIMP_PLUGIN_RESULT_FAILED; }
        zs.next_out = buffer.data_byte();
        zs.avail_out = static_cast<uInt>(buffer.size());
    }
    return BLIMP_PLUGIN_RESULT_OK;
}

BlimpFileChunk BlimpPluginCompressionState::get_compressed_chunk()
{
    if (!available_buffers.empty()) {
        free_buffers.emplace_back(std::move(public_buffer));
        public_buffer = std::move(available_buffers.back());
        available_buffers.pop_back();
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

