#include <compression_zlib.hpp>

#include <catch.hpp>

#include <algorithm>
#include <iterator>
#include <unordered_map>
#include <string>
#include <vector>

struct BlimpKeyValueStoreState {
    std::unordered_map<std::string, std::string> storage;
    int call_count_store;
    int call_count_retrieve;
    std::string last_key_retrieve;

    BlimpKeyValueStoreState()
        :call_count_store(0), call_count_retrieve(0)
    {}

    void store(std::string const& key, BlimpKeyValueStoreValue const& v)
    {
        ++call_count_store;
        storage.insert(std::make_pair(key, std::string(v.data, v.data + v.size)));
    }

    BlimpKeyValueStoreValue retrieve(std::string const& key)
    {
        ++call_count_retrieve;
        last_key_retrieve = key;
        auto it = storage.find(key);
        return (it != storage.end()) ?
            BlimpKeyValueStoreValue{ .data = it->second.data(), .size = static_cast<int64_t>(it->second.size()) } :
            BlimpKeyValueStoreValue{ .data = nullptr, .size = -1 };
    }

    operator BlimpKeyValueStore() {
        return BlimpKeyValueStore {
            .state = this,
            .store = [](BlimpKeyValueStoreStateHandle state, char const* key, BlimpKeyValueStoreValue value)
            {
                return state->store(key, value);
        },
            .retrieve = [](BlimpKeyValueStoreStateHandle state, char const* key) -> BlimpKeyValueStoreValue
        {
            return state->retrieve(key);
        }
        };
    }
};

TEST_CASE("Plugin Compression zlib")
{
    BlimpKeyValueStoreState stub_kv_store;
    SECTION("Plugin Info")
    {
        auto const api_info = blimp_plugin_api_info();
        CHECK(api_info.type == BLIMP_PLUGIN_TYPE_COMPRESSION);
    }

    SECTION("Compression Decompression")
    {
        BlimpKeyValueStoreState kv_store;
        BlimpPluginCompression compression;
        compression.abi = BLIMP_PLUGIN_ABI_1_0_0;
        REQUIRE(blimp_plugin_compression_initialize(kv_store, &compression) == BLIMP_PLUGIN_RESULT_OK);

        char const text[] = "Call me Ishmael. Some years ago -- never mind how long precisely -- having little "
            "or no money in my purse, and nothing particular to interest me on shore, I thought I would sail about a "
            "little and see the watery part of the world. It is a way I have of driving off the spleen, and "
            "regulating the circulation. Whenever I find myself growing grim about the mouth; whenever it is a damp, "
            "drizzly November in my soul; whenever I find myself involuntarily pausing before coffin warehouses, and "
            "bringing up the rear of every funeral I meet; and especially whenever my hypos get such an upper hand "
            "of me, that it requires a strong moral principle to prevent me from deliberately stepping into the "
            "street, and methodically knocking people's hats off -- then, I account it high time to get to sea as "
            "soon as I can. This is my substitute for pistol and ball. With a philosophical flourish Cato throws "
            "himself upon his sword; I quietly take to the ship. There is nothing surprising in this. If they but "
            "knew it, almost all men in their degree, some time or other, cherish very nearly the same feelings "
            "towards the ocean with me.";

        BlimpPluginResult res;
        res = compression.compress_file_chunk(compression.state,
                                              BlimpFileChunk{ .data = text, .size = sizeof(text) });
        CHECK(res == BLIMP_PLUGIN_RESULT_OK);
        res = compression.compress_file_chunk(compression.state,
                                              BlimpFileChunk{ .data = nullptr, .size = 0 });
        CHECK(res == BLIMP_PLUGIN_RESULT_OK);

        std::vector<char> compressed_text;
        BlimpFileChunk c;
        c = compression.get_processed_chunk(compression.state);
        REQUIRE(c.data);
        compressed_text.assign(c.data, c.data + c.size);
        c = compression.get_processed_chunk(compression.state);
        CHECK(!c.data);

        CHECK(compressed_text.size() < sizeof(text));

        res = compression.decompress_file_chunk(compression.state, BlimpFileChunk{ .data = compressed_text.data(), .size = static_cast<int64_t>(compressed_text.size()) });
        CHECK(res == BLIMP_PLUGIN_RESULT_OK);
        res = compression.decompress_file_chunk(compression.state, BlimpFileChunk{ .data = nullptr, .size = 0 });
        CHECK(res == BLIMP_PLUGIN_RESULT_OK);
        std::vector<char> decompressed_text;
        c = compression.get_processed_chunk(compression.state);
        REQUIRE(c.data);
        decompressed_text.assign(c.data, c.data + c.size);
        c = compression.get_processed_chunk(compression.state);
        CHECK(!c.data);
        CHECK(std::equal(decompressed_text.begin(), decompressed_text.end(), text, text + sizeof(text)));


        // decompress in smaller chunks
        REQUIRE(compressed_text.size() > 400);
        res = compression.decompress_file_chunk(compression.state, BlimpFileChunk{ .data = compressed_text.data(), .size = 200 });
        CHECK(res == BLIMP_PLUGIN_RESULT_OK);
        res = compression.decompress_file_chunk(compression.state, BlimpFileChunk{ .data = compressed_text.data() + 200, .size = 200 });
        CHECK(res == BLIMP_PLUGIN_RESULT_OK);
        res = compression.decompress_file_chunk(compression.state, BlimpFileChunk{ .data = compressed_text.data() + 400, .size = static_cast<int64_t>(compressed_text.size() - 400) });
        CHECK(res == BLIMP_PLUGIN_RESULT_OK);
        res = compression.decompress_file_chunk(compression.state, BlimpFileChunk{ .data = nullptr, .size = 0 });
        CHECK(res == BLIMP_PLUGIN_RESULT_OK);

        decompressed_text.clear();
        c = compression.get_processed_chunk(compression.state);
        REQUIRE(c.data);
        decompressed_text.assign(c.data, c.data + c.size);
        c = compression.get_processed_chunk(compression.state);
        CHECK(!c.data);
        CHECK(std::equal(decompressed_text.begin(), decompressed_text.end(), text, text + sizeof(text)));
    }
}
