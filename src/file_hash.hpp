#ifndef BLIMP_INCLUDE_GUARD_FILE_HASH_HPP
#define BLIMP_INCLUDE_GUARD_FILE_HASH_HPP

#include <array>
#include <cstdint>
#include <string>

enum class HashType
{
    SHA_256 = 0
};

struct Hash {
    std::array<std::uint8_t, 32> digest;
};

std::string to_string(Hash const& hash);

#endif
