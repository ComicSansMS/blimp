#ifndef BLIMP_INCLUDE_GUARD_FILE_HASH_HPP
#define BLIMP_INCLUDE_GUARD_FILE_HASH_HPP

#include <array>
#include <cstdint>
#include <memory>
#include <string>

enum class HashType
{
    SHA_256 = 0
};

struct Hash {
    std::array<std::uint8_t, 32> digest;
    static Hash from_string(std::string const& str);
};

class FileChunk;

class FileHasher {
private:
    struct Pimpl;
    std::unique_ptr<Pimpl> m_pimpl;
public:
    explicit FileHasher(HashType hash_type);
    ~FileHasher();
    FileHasher(FileHasher const& rhs);
    FileHasher operator=(FileHasher const& rhs);
    FileHasher(FileHasher&&);
    FileHasher& operator=(FileHasher&&);

    void addData(FileChunk const& chunk);
    Hash getHash();
    void restart();
};

std::string to_string(Hash const& hash);

#endif
