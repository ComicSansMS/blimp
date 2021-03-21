#ifndef BLIMP_INCLUDE_GUARD_FILE_IO_HPP
#define BLIMP_INCLUDE_GUARD_FILE_IO_HPP

#include <file_chunk.hpp>

#include <boost/filesystem/path.hpp>

#include <memory>

class FileIO {
private:
    struct Pimpl;
    std::unique_ptr<Pimpl> m_pimpl;
public:
    FileIO();

    ~FileIO();

    FileIO(FileIO const&) = delete;
    FileIO& operator=(FileIO const&) = delete;

    void startReading(boost::filesystem::path const& p);

    void cancelReading();

    bool hasMoreChunks() const;

    FileChunk const& getNextChunk();
private:
    void scheduleChunkRead();
};

#endif
