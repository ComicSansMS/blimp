#include <storage_filesystem.hpp>

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

TEST_CASE("Plugin Storage Filesystem")
{
    BlimpKeyValueStoreState stub_kv_store;
    SECTION("Plugin Info")
    {
        auto const api_info = blimp_plugin_api_info();
        CHECK(api_info.type == BLIMP_PLUGIN_TYPE_STORAGE);
    }
}
