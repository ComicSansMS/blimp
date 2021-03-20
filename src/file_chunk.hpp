#ifndef BLIMP_INCLUDE_GUARD_FILE_CHUNK_HPP
#define BLIMP_INCLUDE_GUARD_FILE_CHUNK_HPP

#include <cstddef>
#include <vector>

class FileChunk {
private:
    std::vector<char> m_data;
    std::size_t m_used;
public:
    explicit FileChunk(std::size_t chunk_size)
        :m_data(chunk_size, '\0'), m_used(0)
    {}

    FileChunk(FileChunk const&) = default;
    FileChunk& operator=(FileChunk const&) = default;
    FileChunk(FileChunk&&) = default;
    FileChunk& operator=(FileChunk&&) = default;

    std::size_t getChunkSize() const {
        return m_data.size();
    }

    void setUsedSize(std::size_t s) {
        m_used = s;
    }

    std::size_t getUsedSize() const {
        return m_used;
    }

    char* getData() {
        return m_data.data();
    }

    char const* getData() const {
        return m_data.data();
    }
};

#endif
