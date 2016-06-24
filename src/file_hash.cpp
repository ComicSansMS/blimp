#include <file_hash.hpp>

#include <gbBase/Assert.hpp>

#include <hex.h>

std::string to_string(Hash const& hash)
{
    CryptoPP::HexEncoder enc;
    enc.Put(reinterpret_cast<byte const*>(hash.digest.data()), hash.digest.size());
    std::array<char, 2*std::tuple_size<decltype(hash.digest)>::value + 1> buffer;
    auto const res = enc.Get(reinterpret_cast<byte*>(buffer.data()), 2*hash.digest.size());
    GHULBUS_ASSERT(res == 2*hash.digest.size());
    buffer.back() = '\0';
    return std::string(begin(buffer), end(buffer));
}
