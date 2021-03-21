#include <file_io.hpp>

#include <worker_pool.hpp>

#include <gbBase/Assert.hpp>
#include <gbBase/Exception.hpp>
#include <gbBase/Finally.hpp>

#include <cstdio>
#include <vector>

struct FileIO::Pimpl {
    boost::filesystem::path filepath;
    std::vector<FileChunk> chunks;
    std::size_t beginReadyChunk;
    std::size_t endReadyChunk;
    FILE* fin;
    WorkerPool pool;

    Pimpl()
        :beginReadyChunk(0), endReadyChunk(0), fin(nullptr), pool(1)
    {
        chunks.reserve(4);
        for (int i = 0; i < 4; ++i) { chunks.emplace_back(1024*1024); }
    }
};

FileIO::FileIO()
    :m_pimpl(std::make_unique<Pimpl>())
{
}

FileIO::~FileIO()
{
    if (m_pimpl->fin) {
        std::fclose(m_pimpl->fin);
    }
}

void FileIO::startReading(boost::filesystem::path const& p)
{
    m_pimpl->filepath = p;
    m_pimpl->fin = std::fopen(p.string().c_str(), "rb");
    if (!m_pimpl->fin) {
        GHULBUS_THROW(Ghulbus::Exceptions::IOError{} << Ghulbus::Exception_Info::filename(p.string()),
                      "Unable to open file.");
    }
    m_pimpl->pool.schedule(
        []()
        {

        });
}

void FileIO::cancelReading()
{
    std::fclose(m_pimpl->fin);
    m_pimpl->fin = nullptr;
    m_pimpl->filepath = boost::filesystem::path{};
}

bool FileIO::hasMoreChunks() const
{
    return (m_pimpl->fin != nullptr);
}

FileChunk const& FileIO::getNextChunk()
{
    GHULBUS_PRECONDITION(hasMoreChunks());
    FileChunk& chunk = m_pimpl->chunks.front();
    std::size_t read = std::fread(chunk.getData(), 1, chunk.getChunkSize(), m_pimpl->fin);
    if (read < chunk.getChunkSize()) {
        if (std::ferror(m_pimpl->fin) != 0) {
            GHULBUS_THROW(Ghulbus::Exceptions::IOError{} << Ghulbus::Exception_Info::filename(m_pimpl->filepath.string()),
                          "Error reading from file.");
        } else if (std::feof(m_pimpl->fin) != 0) {
            std::fclose(m_pimpl->fin);
            m_pimpl->fin = nullptr;
            m_pimpl->filepath = boost::filesystem::path{};
        } else {
            GHULBUS_THROW(Ghulbus::Exceptions::IOError{} << Ghulbus::Exception_Info::filename(m_pimpl->filepath.string()),
                          "Unexpected I/O error.");
        }
    }
    chunk.setUsedSize(read);
    return chunk;
}
