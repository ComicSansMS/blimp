#include <file_hash.hpp>

#include <file_chunk.hpp>

#include <gbBase/Assert.hpp>
#include <gbBase/Exception.hpp>

#include <hex.h>
#include <sha.h>

#include <condition_variable>
#include <mutex>
#include <thread>

struct FileHasher::Pimpl {
    CryptoPP::SHA256 hasher;
    bool has_cached_hash;
    Hash cached_hash;

    Pimpl()
        :has_cached_hash(false)
    {}
};

FileHasher::FileHasher(HashType hash_type)
{
    GHULBUS_PRECONDITION(hash_type == HashType::SHA_256);
    m_pimpl = std::make_unique<Pimpl>();
}

FileHasher::~FileHasher() = default;
FileHasher::FileHasher(FileHasher&&) = default;
FileHasher& FileHasher::operator=(FileHasher&&) = default;

FileHasher::FileHasher(FileHasher const& rhs)
    :m_pimpl(std::make_unique<Pimpl>(*rhs.m_pimpl))
{}

FileHasher FileHasher::operator=(FileHasher const& rhs)
{
    if (&rhs != this) {
        *m_pimpl = *rhs.m_pimpl;
    }
    return *this;
}

void FileHasher::addData(FileChunk const& chunk)
{
    m_pimpl->hasher.Update(reinterpret_cast<CryptoPP::byte const*>(chunk.getData()), chunk.getUsedSize());
}

Hash FileHasher::getHash()
{
    if (!m_pimpl->has_cached_hash) {
        CryptoPP::SHA256 cc = m_pimpl->hasher;
        m_pimpl->hasher.Final(reinterpret_cast<CryptoPP::byte*>(m_pimpl->cached_hash.digest.data()));
        m_pimpl->has_cached_hash = true;
    }
    return m_pimpl->cached_hash;
}

void FileHasher::restart()
{
    m_pimpl->hasher.Restart();
    m_pimpl->has_cached_hash = false;
}

std::string to_string(Hash const& hash)
{
    CryptoPP::HexEncoder enc;
    enc.Put(reinterpret_cast<CryptoPP::byte const*>(hash.digest.data()), hash.digest.size());
    std::array<char, 2*std::tuple_size<decltype(hash.digest)>::value + 7> buffer;
    buffer[0] = 'S';
    buffer[1] = 'H';
    buffer[2] = 'A';
    buffer[3] = '2';
    buffer[4] = '5';
    buffer[5] = '6';
    buffer[6] = ':';
    auto const res = enc.Get(reinterpret_cast<CryptoPP::byte*>(buffer.data() + 7), 2*hash.digest.size());
    GHULBUS_ASSERT(res == 2*hash.digest.size());
    return std::string(begin(buffer), end(buffer));
}

Hash Hash::from_string(std::string const& str)
{
    if (!str.starts_with("SHA256:")) {
        GHULBUS_THROW(Ghulbus::Exceptions::IOError{}, "Unable to parse Hash from string");
    }
    CryptoPP::HexDecoder dec;
    dec.Put(reinterpret_cast<CryptoPP::byte const*>(str.c_str() + 7), str.length() - 7);
    Hash ret;
    auto const res = dec.Get(reinterpret_cast<CryptoPP::byte*>(&ret.digest), ret.digest.size());
    if (res != ret.digest.size()) {
        GHULBUS_THROW(Ghulbus::Exceptions::IOError{}, "Unable to parse Hash from string");
    }
    return ret;
}
