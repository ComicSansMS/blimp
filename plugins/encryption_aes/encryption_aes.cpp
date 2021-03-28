#include <encryption_aes.hpp>

#include <blimp_plugin_helper_cpp.hpp>

#include <aes.h>
#include <filters.h>
#include <hex.h>
#include <modes.h>
#include <osrng.h>
#include <pwdbased.h>
#include <sha.h>

#include <algorithm>
#include <array>
#include <deque>
#include <stdexcept>
#include <vector>

namespace {
struct ErrorStrings {
    static constexpr char const okay[] = "Ok";
    static constexpr char const encryption_error[] = "Unexpected error during encryption";
    static constexpr char const corrupted_database_master_key[] = "Master key entry in database is corrupted";
    static constexpr char const corrupted_database_container_key[] = "Container key entry in database is corrupted";
    static constexpr char const corrupted_database_container_iv[] = "Container initialization vector entry in database is corrupted";
    static constexpr char const master_key_error[] = "Error reconstructing master key";
};

std::string to_string(CryptoPP::byte const* data, std::size_t size)
{
    CryptoPP::HexEncoder enc;
    enc.Put(data, size);
    std::vector<char> out_buffer;
    out_buffer.resize(2 * size);
    auto const res = enc.Get(reinterpret_cast<CryptoPP::byte*>(out_buffer.data()), out_buffer.size());
    return std::string(begin(out_buffer), end(out_buffer));
}

std::vector<CryptoPP::byte> from_string(char const* data, std::size_t size)
{
    CryptoPP::HexDecoder dec;
    dec.Put(reinterpret_cast<CryptoPP::byte const*>(data), size);
    std::vector<CryptoPP::byte> ret;
    ret.resize(size/2);
    dec.Get(ret.data(), ret.size());
    return ret;
}
}   // anonymous namespace

struct BlimpPluginEncryptionState {
    char const* error_string;
    KeyValueStore kv_store;
    CryptoPP::AutoSeededRandomPool pool;
    CryptoPP::AutoSeededRandomPool iv_pool;
    std::array<CryptoPP::byte, CryptoPP::AES::MAX_KEYLENGTH> master_key;
    CryptoPP::ECB_Mode<CryptoPP::AES>::Encryption master_encryption;
    CryptoPP::ECB_Mode<CryptoPP::AES>::Decryption master_decryption;
    std::array<CryptoPP::byte, CryptoPP::AES::MAX_KEYLENGTH> container_key;
    CryptoPP::CBC_Mode<CryptoPP::AES>::Encryption container_encryption;
    std::vector<CryptoPP::byte> in_buffer;
    std::deque<std::vector<CryptoPP::byte>> out_available;
    std::vector<std::vector<CryptoPP::byte>> out_free;
    std::vector<CryptoPP::byte> busy_buffer;

    BlimpPluginEncryptionState(BlimpKeyValueStore const& n_kv_store);
    ~BlimpPluginEncryptionState();

    BlimpPluginEncryptionState(BlimpPluginEncryptionState const&) = delete;
    BlimpPluginEncryptionState& operator=(BlimpPluginEncryptionState const&) = delete;

    char const* get_last_error();
    int32_t get_block_size();

    BlimpPluginResult set_password(BlimpPluginEncryptionPassword const& password);
    BlimpPluginResult new_storage_container(int64_t container_id);
    BlimpPluginResult encrypt_file_chunk(BlimpFileChunk const& file_chunk);
    BlimpFileChunk get_encrypted_chunk();

    std::vector<CryptoPP::byte> getFreeBuffer(std::size_t s);
};

BlimpPluginInfo blimp_plugin_api_info()
{
    return BlimpPluginInfo{
        .type = BLIMP_PLUGIN_TYPE_ENCRYPTION,
        .version = BlimpPluginVersion{
            .major = 1,
            .minor = 0,
            .patch = 0
    },
        .uuid = {
            // {2CCA3B08-97F3-4710-B8CA-1E99B8C56AA7}
            0x2cca3b08, 0x97f3, 0x4710, { 0xb8, 0xca, 0x1e, 0x99, 0xb8, 0xc5, 0x6a, 0xa7 }
        },
        .name = "AES Encryption",
        .description = "AES Encryption with Crypto++"
    };
}

char const* blimp_plugin_get_last_error(BlimpPluginEncryptionStateHandle state)
{
    return state->get_last_error();
}

int32_t blimp_plugin_get_block_size(BlimpPluginEncryptionStateHandle state)
{
    return state->get_block_size();
}

BlimpPluginResult blimp_plugin_set_password(BlimpPluginEncryptionStateHandle state, BlimpPluginEncryptionPassword password)
{
    return state->set_password(password);
}

BlimpPluginResult blimp_plugin_new_storage_container(BlimpPluginEncryptionStateHandle state, int64_t container_id)
{
    return state->new_storage_container(container_id);
}

BlimpPluginResult blimp_plugin_encrypt_file_chunk(BlimpPluginEncryptionStateHandle state, BlimpFileChunk file_chunk)
{
    return state->encrypt_file_chunk(file_chunk);
}

BlimpFileChunk blimp_plugin_get_encrypted_chunk(BlimpPluginEncryptionStateHandle state)
{
    return state->get_encrypted_chunk();
}

BlimpPluginResult blimp_plugin_encryption_initialize(BlimpKeyValueStore kv_store, BlimpPluginEncryption* plugin)
{
    if (plugin->abi != BLIMP_PLUGIN_ABI_1_0_0) {
        return BLIMP_PLUGIN_RESULT_INVALID_ARGUMENT;
    }
    try {
        plugin->state = new BlimpPluginEncryptionState(kv_store);
    } catch (std::exception&) {
        plugin->state = nullptr;
        return BLIMP_PLUGIN_RESULT_FAILED;
    }
    plugin->get_last_error = blimp_plugin_get_last_error;
    plugin->get_block_size = blimp_plugin_get_block_size;
    plugin->set_password = blimp_plugin_set_password;
    plugin->new_storage_container = blimp_plugin_new_storage_container;
    plugin->encrypt_file_chunk = blimp_plugin_encrypt_file_chunk;
    plugin->get_encrypted_chunk = blimp_plugin_get_encrypted_chunk;
    return BLIMP_PLUGIN_RESULT_OK;
}

void blimp_plugin_encryption_shutdown(BlimpPluginEncryption* plugin)
{
    delete plugin->state;
}

BlimpPluginEncryptionState::BlimpPluginEncryptionState(BlimpKeyValueStore const& n_kv_store)
    :kv_store(n_kv_store), master_key{ 0 }, container_key{ 0 }
{
    error_string = ErrorStrings::okay;
}

BlimpPluginEncryptionState::~BlimpPluginEncryptionState()
{
    std::fill(master_key.begin(), master_key.end(), 0);
    std::fill(container_key.begin(), container_key.end(), 0);
}

char const* BlimpPluginEncryptionState::get_last_error()
{
    return error_string;
}

int32_t BlimpPluginEncryptionState::get_block_size()
{
    return container_encryption.MandatoryBlockSize();
}

BlimpPluginResult BlimpPluginEncryptionState::set_password(BlimpPluginEncryptionPassword const& password)
{
    BlimpKeyValueStoreValue salt_v = kv_store.retrieve("master_salt");
    if (salt_v.data == nullptr) {
        std::array<CryptoPP::byte, 32> salt;
        pool.GenerateBlock(salt.data(), salt.size());
        auto const master_salt_str = to_string(salt.data(), salt.size());
        BlimpKeyValueStoreValue v;
        v.data = master_salt_str.data();
        v.size = master_salt_str.size();
        kv_store.store("master_salt", v);
        salt_v = kv_store.retrieve("master_salt");
    }
    if (salt_v.size != 64) {
        error_string = ErrorStrings::corrupted_database_master_key;
        return BLIMP_PLUGIN_RESULT_CORRUPTED_DATA;
    }

    unsigned int const n_iterations = 97'495;
    CryptoPP::PKCS5_PBKDF2_HMAC<CryptoPP::SHA256> pbkdf;
    auto const salt = from_string(salt_v.data, salt_v.size);
    auto const res = pbkdf.DeriveKey(master_key.data(), master_key.size(), 0,
                                     reinterpret_cast<CryptoPP::byte const*>(password.data), password.size,
                                     salt.data(), salt.size(), n_iterations);
    master_encryption.SetKey(master_key.data(), master_key.size());
    master_decryption.SetKey(master_key.data(), master_key.size());
    if (res != n_iterations) { error_string = ErrorStrings::master_key_error; return BLIMP_PLUGIN_RESULT_FAILED; }
    return BLIMP_PLUGIN_RESULT_OK;
}

BlimpPluginResult BlimpPluginEncryptionState::new_storage_container(int64_t container_id)
{
    std::string const kv_string_key = "container_key_" + std::to_string(container_id);
    BlimpKeyValueStoreValue container_key_v = kv_store.retrieve(kv_string_key.c_str());
    if (container_key_v.data == nullptr) {
        std::array<CryptoPP::byte, CryptoPP::AES::MAX_KEYLENGTH> new_key;
        pool.GenerateBlock(new_key.data(), new_key.size());
        std::array<CryptoPP::byte, CryptoPP::AES::MAX_KEYLENGTH> encrypted_key;
        master_encryption.ProcessData(encrypted_key.data(), new_key.data(), new_key.size());
        auto const encrypted_key_str = to_string(encrypted_key.data(), encrypted_key.size());
        BlimpKeyValueStoreValue const v{
            .data = encrypted_key_str.data(), .size = static_cast<int64_t>(encrypted_key_str.size())
        };
        kv_store.store(kv_string_key.c_str(), v);
        std::fill(new_key.begin(), new_key.end(), 0);
        container_key_v = kv_store.retrieve(kv_string_key.c_str());
    }
    if (container_key_v.size != CryptoPP::AES::MAX_KEYLENGTH * 2) {
        error_string = ErrorStrings::corrupted_database_container_key;
        return BLIMP_PLUGIN_RESULT_CORRUPTED_DATA;
    }

    std::string const kv_string_iv = "container_iv_" + std::to_string(container_id);
    BlimpKeyValueStoreValue container_iv_v = kv_store.retrieve(kv_string_iv.c_str());
    if (container_iv_v.data == nullptr) {
        std::array<CryptoPP::byte, CryptoPP::AES::BLOCKSIZE> new_iv;
        iv_pool.GenerateBlock(new_iv.data(), new_iv.size());
        auto const iv_str = to_string(new_iv.data(), new_iv.size());
        BlimpKeyValueStoreValue const v{ .data = iv_str.data(), .size = static_cast<int64_t>(iv_str.size()) };
        kv_store.store(kv_string_iv.c_str(), v);
        container_iv_v = kv_store.retrieve(kv_string_iv.c_str());
    }
    if (container_iv_v.size != CryptoPP::AES::BLOCKSIZE * 2) {
        error_string = ErrorStrings::corrupted_database_container_iv;
        return BLIMP_PLUGIN_RESULT_CORRUPTED_DATA;
    }

    auto const encrypted_key = from_string(container_key_v.data, container_key_v.size);
    master_decryption.ProcessData(container_key.data(), encrypted_key.data(), encrypted_key.size());

    auto const container_iv = from_string(container_iv_v.data, container_iv_v.size);
    container_encryption.SetKeyWithIV(container_key.data(), container_key.size(), container_iv.data());

    return BLIMP_PLUGIN_RESULT_OK;
}

BlimpPluginResult BlimpPluginEncryptionState::encrypt_file_chunk(BlimpFileChunk const& file_chunk)
{
    std::vector<CryptoPP::byte> enc_buffer = getFreeBuffer(in_buffer.size() + file_chunk.size);
    auto const it_buffer = std::copy(in_buffer.begin(), in_buffer.end(), enc_buffer.begin());
    if (file_chunk.data != nullptr) {
        std::copy(file_chunk.data, file_chunk.data + file_chunk.size, it_buffer);
        std::size_t const overflow = enc_buffer.size() % CryptoPP::AES::BLOCKSIZE;
        in_buffer.assign(enc_buffer.begin() + (enc_buffer.size() - overflow), enc_buffer.end());
        enc_buffer.resize(enc_buffer.size() - overflow);
    } else {
        in_buffer.clear();
        std::size_t const padding = CryptoPP::AES::BLOCKSIZE - (enc_buffer.size() % CryptoPP::AES::BLOCKSIZE);
        if ((padding != 0) && (padding != CryptoPP::AES::BLOCKSIZE)) {
            enc_buffer.resize(enc_buffer.size() + padding);
            pool.GenerateBlock(enc_buffer.data() + (enc_buffer.size() - padding), padding);
        }
    }

    if (!enc_buffer.empty()) {
        container_encryption.ProcessData(enc_buffer.data(), enc_buffer.data(), enc_buffer.size());
        out_available.emplace_back(std::move(enc_buffer));
    }

    return BLIMP_PLUGIN_RESULT_OK;
}

BlimpFileChunk BlimpPluginEncryptionState::get_encrypted_chunk()
{
    BlimpFileChunk ret{};
    if (!out_available.empty()) {
        busy_buffer = std::move(out_available.front());
        out_available.pop_front();
        ret.data = reinterpret_cast<char const*>(busy_buffer.data());
        ret.size = busy_buffer.size();
    }
    return ret;
}

std::vector<CryptoPP::byte> BlimpPluginEncryptionState::getFreeBuffer(std::size_t s)
{
    if (out_free.empty()) {
        return std::vector<CryptoPP::byte>(s, 0);
    } else {
        std::vector<CryptoPP::byte> ret = std::move(out_free.back());
        out_free.pop_back();
        ret.resize(s);
        return ret;
    }
}
