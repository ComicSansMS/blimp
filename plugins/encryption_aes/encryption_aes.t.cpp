#include <encryption_aes.hpp>

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

TEST_CASE("Plugin Encryption AES")
{
    BlimpKeyValueStoreState stub_kv_store;
    SECTION("Plugin Info")
    {
        auto const api_info = blimp_plugin_api_info();
        CHECK(api_info.type == BLIMP_PLUGIN_TYPE_ENCRYPTION);
    }

    SECTION("Encrypt-Decrypt Round-trip")
    {
        BlimpPluginEncryption plugin{};
        plugin.abi = BLIMP_PLUGIN_ABI_1_0_0;
        REQUIRE(blimp_plugin_encryption_initialize(stub_kv_store, &plugin) == BLIMP_PLUGIN_RESULT_OK);
        REQUIRE(plugin.state != nullptr);
        REQUIRE(plugin.get_last_error != nullptr);
        REQUIRE(plugin.set_password != nullptr);
        REQUIRE(plugin.new_storage_container != nullptr);
        REQUIRE(plugin.encrypt_file_chunk != nullptr);
        REQUIRE(plugin.decrypt_file_chunk != nullptr);
        REQUIRE(plugin.get_processed_chunk != nullptr);

        char const sample_password[] = "correcthorsebatterystaple";
        BlimpPluginEncryptionPassword const blimp_password{ .data = sample_password,
                                                            .size = sizeof(sample_password) };
        CHECK(!stub_kv_store.storage.contains("master_salt"));
        REQUIRE(plugin.set_password(plugin.state, blimp_password) == BLIMP_PLUGIN_RESULT_OK);

        // Set Password saves master key salt to KV store
        {
            REQUIRE(stub_kv_store.storage.contains("master_salt"));
            CHECK(stub_kv_store.storage["master_salt"].size() == 64);
        }

        CHECK(!stub_kv_store.storage.contains("container_key_4711"));
        CHECK(!stub_kv_store.storage.contains("container_iv_4711"));
        REQUIRE(plugin.new_storage_container(plugin.state, 4711) == BLIMP_PLUGIN_RESULT_OK);
        // Storage Container Secret Key and IV saved to KV Store
        {
            REQUIRE(stub_kv_store.storage.contains("container_key_4711"));
            REQUIRE(stub_kv_store.storage.contains("container_iv_4711"));
            CHECK(stub_kv_store.storage["container_key_4711"].size() == 64);
            CHECK(stub_kv_store.storage["container_iv_4711"].size() == 32);
        }

        /// Storage Container Secret Key and IV stay the same for same containers, but not for different containers
        {
            auto const original_key = stub_kv_store.storage["container_key_4711"];
            auto const original_iv = stub_kv_store.storage["container_iv_4711"];
            CHECK(plugin.new_storage_container(plugin.state, 12345) == BLIMP_PLUGIN_RESULT_OK);
            CHECK(stub_kv_store.storage["container_key_12345"].size() == 64);
            CHECK(stub_kv_store.storage["container_iv_12345"].size() == 32);
            CHECK(stub_kv_store.storage["container_key_12345"] != original_key);
            CHECK(stub_kv_store.storage["container_iv_12345"] != original_iv);

            CHECK(plugin.new_storage_container(plugin.state, 4711) == BLIMP_PLUGIN_RESULT_OK);
            CHECK(stub_kv_store.storage["container_key_4711"] == original_key);
            CHECK(stub_kv_store.storage["container_iv_4711"] == original_iv);
        }

        SECTION("Single Block")
        {
            char const plaintext[] = "One single block";
            REQUIRE(sizeof(plaintext) == 17);
            REQUIRE(plaintext[16] == '\0');
            CHECK(plugin.encrypt_file_chunk(plugin.state, BlimpFileChunk{ .data = plaintext, .size = 16 }) == BLIMP_PLUGIN_RESULT_OK);
            CHECK(plugin.encrypt_file_chunk(plugin.state, BlimpFileChunk{ .data= nullptr, .size = 0 }) == BLIMP_PLUGIN_RESULT_OK);
            std::vector<char> encrypted_data;
            BlimpFileChunk c;
            c = plugin.get_processed_chunk(plugin.state);
            REQUIRE(c.data != nullptr);
            REQUIRE(c.size == 32);
            std::copy(c.data, c.data + c.size, back_inserter(encrypted_data));
            c = plugin.get_processed_chunk(plugin.state);
            CHECK(c.data == nullptr);
            CHECK(c.size == 0);

            SECTION("Decrypt all at once")
            {
                CHECK(plugin.decrypt_file_chunk(plugin.state, BlimpFileChunk{ .data = encrypted_data.data(), .size = static_cast<int64_t>(encrypted_data.size()) }) == BLIMP_PLUGIN_RESULT_OK);
                CHECK(plugin.decrypt_file_chunk(plugin.state, BlimpFileChunk{ .data = nullptr, .size = 0 }) == BLIMP_PLUGIN_RESULT_OK);
                std::vector<char> decrypted_data;
                c = plugin.get_processed_chunk(plugin.state);
                REQUIRE(c.data != nullptr);
                REQUIRE(c.size == 16);
                std::copy(c.data, c.data + c.size, back_inserter(decrypted_data));
                c = plugin.get_processed_chunk(plugin.state);
                CHECK(c.data == nullptr);
                CHECK(c.size == 0);
                CHECK(std::equal(plaintext, plaintext + 16, begin(decrypted_data), end(decrypted_data)));
            }

            SECTION("Decrypt in several runs")
            {
                CHECK(plugin.decrypt_file_chunk(plugin.state, BlimpFileChunk{ .data = encrypted_data.data(), .size = 1 }) == BLIMP_PLUGIN_RESULT_OK);
                CHECK(plugin.decrypt_file_chunk(plugin.state, BlimpFileChunk{ .data = encrypted_data.data() + 1, .size = 2 }) == BLIMP_PLUGIN_RESULT_OK);
                CHECK(plugin.decrypt_file_chunk(plugin.state, BlimpFileChunk{ .data = encrypted_data.data() + 3, .size = 20 }) == BLIMP_PLUGIN_RESULT_OK);
                CHECK(plugin.decrypt_file_chunk(plugin.state, BlimpFileChunk{ .data = encrypted_data.data() + 23, .size = 7 }) == BLIMP_PLUGIN_RESULT_OK);
                CHECK(plugin.decrypt_file_chunk(plugin.state, BlimpFileChunk{ .data = encrypted_data.data() + 30, .size = 1 }) == BLIMP_PLUGIN_RESULT_OK);
                CHECK(plugin.decrypt_file_chunk(plugin.state, BlimpFileChunk{ .data = encrypted_data.data() + 31, .size = 1 }) == BLIMP_PLUGIN_RESULT_OK);
                CHECK(plugin.decrypt_file_chunk(plugin.state, BlimpFileChunk{ .data = nullptr, .size = 0 }) == BLIMP_PLUGIN_RESULT_OK);
                std::vector<char> decrypted_data;
                c = plugin.get_processed_chunk(plugin.state);
                REQUIRE(c.data != nullptr);
                REQUIRE(c.size == 16);
                std::copy(c.data, c.data + c.size, back_inserter(decrypted_data));
                c = plugin.get_processed_chunk(plugin.state);
                CHECK(c.data == nullptr);
                CHECK(c.size == 0);
                CHECK(std::equal(plaintext, plaintext + 16, begin(decrypted_data), end(decrypted_data)));
            }

            SECTION("Encrypting same data twice gives different results in CBC mode")
            {
                CHECK(plugin.encrypt_file_chunk(plugin.state, BlimpFileChunk{ .data = plaintext, .size = 16 }) == BLIMP_PLUGIN_RESULT_OK);
                CHECK(plugin.encrypt_file_chunk(plugin.state, BlimpFileChunk{ .data= nullptr, .size = 0 }) == BLIMP_PLUGIN_RESULT_OK);
                c = plugin.get_processed_chunk(plugin.state);
                REQUIRE(c.data != nullptr);
                REQUIRE(c.size == 32);
                // only compare first 16 bytes, as the rest is padding
                CHECK(!std::equal(c.data, c.data + 16, encrypted_data.data()));

                // both still decrypt to the same plaintext, 
                std::vector<char> encrypted2;
                std::copy(c.data, c.data + c.size, back_inserter(encrypted2));

                CHECK(plugin.get_processed_chunk(plugin.state).data == nullptr);
                CHECK(plugin.decrypt_file_chunk(plugin.state, BlimpFileChunk{ .data = encrypted_data.data(), .size = static_cast<int64_t>(encrypted_data.size()) }) == BLIMP_PLUGIN_RESULT_OK);
                CHECK(plugin.decrypt_file_chunk(plugin.state, BlimpFileChunk{ .data = nullptr, .size = 0 }) == BLIMP_PLUGIN_RESULT_OK);
                CHECK(plugin.decrypt_file_chunk(plugin.state, BlimpFileChunk{ .data = encrypted2.data(), .size = static_cast<int64_t>(encrypted2.size()) }) == BLIMP_PLUGIN_RESULT_OK);
                CHECK(plugin.decrypt_file_chunk(plugin.state, BlimpFileChunk{ .data = nullptr, .size = 0 }) == BLIMP_PLUGIN_RESULT_OK);
                c = plugin.get_processed_chunk(plugin.state);
                REQUIRE(c.data != nullptr);
                CHECK(std::equal(c.data, c.data + c.size, plaintext, plaintext + 16));
                c = plugin.get_processed_chunk(plugin.state);
                REQUIRE(c.data != nullptr);
                CHECK(std::equal(c.data, c.data + c.size, plaintext, plaintext + 16));
                CHECK(plugin.get_processed_chunk(plugin.state).data == nullptr);
            }

            SECTION("Encrypt in several runs")
            {
                // reset encoder state
                plugin.new_storage_container(plugin.state, 4711);
                std::vector<char> encrypted2;
                CHECK(plugin.encrypt_file_chunk(plugin.state, BlimpFileChunk{ .data = plaintext, .size = 1 }) == BLIMP_PLUGIN_RESULT_OK);
                CHECK(plugin.encrypt_file_chunk(plugin.state, BlimpFileChunk{ .data = plaintext + 1, .size = 5 }) == BLIMP_PLUGIN_RESULT_OK);
                CHECK(plugin.encrypt_file_chunk(plugin.state, BlimpFileChunk{ .data = plaintext + 6, .size = 8 }) == BLIMP_PLUGIN_RESULT_OK);
                CHECK(plugin.encrypt_file_chunk(plugin.state, BlimpFileChunk{ .data = plaintext + 14, .size = 1 }) == BLIMP_PLUGIN_RESULT_OK);
                CHECK(plugin.encrypt_file_chunk(plugin.state, BlimpFileChunk{ .data = plaintext + 15, .size = 1 }) == BLIMP_PLUGIN_RESULT_OK);
                c = plugin.get_processed_chunk(plugin.state);
                CHECK(c.data == nullptr);
                CHECK(c.size == 0);
                CHECK(plugin.encrypt_file_chunk(plugin.state, BlimpFileChunk{ .data= nullptr, .size = 0 }) == BLIMP_PLUGIN_RESULT_OK);
                c = plugin.get_processed_chunk(plugin.state);
                REQUIRE(c.data != nullptr);
                REQUIRE(c.size == 32);
                std::copy(c.data, c.data + c.size, back_inserter(encrypted2));

                // first 16 bytes are equal, as they contain same data
                CHECK(std::equal(encrypted2.begin(), encrypted2.begin() + 16, encrypted_data.begin()));
                // but padding bytes are random, so the rest of the data is different
                CHECK(encrypted2 != encrypted_data);

                // it still decodes to the same plaintext though
                CHECK(plugin.decrypt_file_chunk(plugin.state, BlimpFileChunk{ .data= encrypted2.data(), .size = static_cast<int64_t>(encrypted2.size()) }) == BLIMP_PLUGIN_RESULT_OK);
                CHECK(plugin.decrypt_file_chunk(plugin.state, BlimpFileChunk{ .data= nullptr, .size = 0 }) == BLIMP_PLUGIN_RESULT_OK);
                c = plugin.get_processed_chunk(plugin.state);
                REQUIRE(c.data != nullptr);
                REQUIRE(c.size == 16);
                CHECK(std::equal(c.data, c.data + c.size, plaintext));
            }
        }

        SECTION("Single Byte")
        {
            char const plainchar = '!';
            CHECK(plugin.encrypt_file_chunk(plugin.state, BlimpFileChunk{ .data = &plainchar, .size = 1 }) == BLIMP_PLUGIN_RESULT_OK);
            CHECK(plugin.encrypt_file_chunk(plugin.state, BlimpFileChunk{ .data = nullptr, .size = 0 }) == BLIMP_PLUGIN_RESULT_OK);
            BlimpFileChunk c;
            c = plugin.get_processed_chunk(plugin.state);
            REQUIRE(c.data);
            REQUIRE(c.size == 16);
            std::vector encrypted_data(c.data, c.data + c.size);
            CHECK(!plugin.get_processed_chunk(plugin.state).data);

            CHECK(plugin.decrypt_file_chunk(plugin.state, BlimpFileChunk{ .data = encrypted_data.data(), .size = 16 }) == BLIMP_PLUGIN_RESULT_OK);
            CHECK(plugin.decrypt_file_chunk(plugin.state, BlimpFileChunk{ .data = nullptr, .size = 0 }) == BLIMP_PLUGIN_RESULT_OK);
            c = plugin.get_processed_chunk(plugin.state);
            REQUIRE(c.data);
            REQUIRE(c.size == 1);
            CHECK(c.data[0] == plainchar);
            CHECK(!plugin.get_processed_chunk(plugin.state).data);
        }

        SECTION("Larger block of data")
        {
            char const plaintext[] =
                "I find it wholesome to be alone the greater part of the time. To be in company, even with the best, "
                "is soon wearisome and dissipating. I love to be alone. I never found the companion that was so "
                "companionable as solitude. We are for the most part more lonely when we go abroad among men than "
                "when we stay in our chambers. A man thinking or working is always alone, let him be where he will. "
                "Solitude is not measured by the miles of space that intervene between a man and his fellows. "
                "The really diligent student in one of the crowded hives of Cambridge College is as solitary as a "
                "dervish in the desert.";
            CHECK(plugin.encrypt_file_chunk(plugin.state, BlimpFileChunk{ .data = plaintext, .size = sizeof(plaintext) }) == BLIMP_PLUGIN_RESULT_OK);
            CHECK(plugin.encrypt_file_chunk(plugin.state, BlimpFileChunk{ .data = nullptr, .size = 0 }) == BLIMP_PLUGIN_RESULT_OK);
            BlimpFileChunk c;
            c = plugin.get_processed_chunk(plugin.state);
            std::vector<char> encrypted_data(c.data, c.data + c.size);
            c = plugin.get_processed_chunk(plugin.state);
            std::copy(c.data, c.data + c.size, back_inserter(encrypted_data));
            CHECK(!plugin.get_processed_chunk(plugin.state).data);

            CHECK(plugin.decrypt_file_chunk(plugin.state, BlimpFileChunk{ .data = encrypted_data.data(), .size = static_cast<int64_t>(encrypted_data.size()) }) == BLIMP_PLUGIN_RESULT_OK);
            CHECK(plugin.decrypt_file_chunk(plugin.state, BlimpFileChunk{ .data = nullptr, .size = 0 }) == BLIMP_PLUGIN_RESULT_OK);
            c = plugin.get_processed_chunk(plugin.state);
            std::vector<char> decrypted_data(c.data, c.data + c.size);
            c = plugin.get_processed_chunk(plugin.state);
            std::copy(c.data, c.data + c.size, back_inserter(decrypted_data));
            CHECK(!plugin.get_processed_chunk(plugin.state).data);
            CHECK(std::equal(decrypted_data.begin(), decrypted_data.end(), plaintext, plaintext + sizeof(plaintext)));
        }

        blimp_plugin_encryption_shutdown(&plugin);
    }
}
