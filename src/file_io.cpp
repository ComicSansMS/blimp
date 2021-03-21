#include <file_io.hpp>

#include <worker_pool.hpp>

#include <gbBase/Assert.hpp>
#include <gbBase/Exception.hpp>
#include <gbBase/Finally.hpp>

#include <cstdio>
#include <future>
#include <limits>
#include <tuple>
#include <vector>

struct FileIO::Pimpl {
    boost::filesystem::path filepath;
    std::vector<FileChunk> chunks;
    std::size_t beginReadyChunk;
    FILE* fin;
    WorkerPool pool;
    std::deque<std::future<std::tuple<std::size_t, bool>>> outstanding_reads;

    Pimpl()
        :beginReadyChunk(0), fin(nullptr), pool(1)
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
    if (!m_pimpl->outstanding_reads.empty()) {
        cancelReading();
    }
}

void FileIO::startReading(boost::filesystem::path const& p)
{
    GHULBUS_PRECONDITION(!hasMoreChunks());
    m_pimpl->fin = std::fopen(p.string().c_str(), "rb");
    if (!m_pimpl->fin) {
        GHULBUS_THROW(Ghulbus::Exceptions::IOError{} << Ghulbus::Exception_Info::filename(p.string()),
                      "Unable to open file.");
    }
    m_pimpl->filepath = p;

    for (std::size_t i = 0, i_end = m_pimpl->chunks.size() - 1; i < i_end; ++i) {
        scheduleChunkRead();
    }
}

void FileIO::scheduleChunkRead()
{
    std::size_t const index = m_pimpl->beginReadyChunk;
    m_pimpl->beginReadyChunk = (m_pimpl->beginReadyChunk + 1) % m_pimpl->chunks.size();

    std::packaged_task<std::tuple<std::size_t, bool>()> pt{
        [this, index]() -> std::tuple<std::size_t, bool> {
            bool do_continue = true;
            if (!m_pimpl->fin) { return std::make_tuple(std::numeric_limits<std::size_t>::max(), false); }
            FileChunk& chunk = m_pimpl->chunks[index];
            std::size_t read = std::fread(chunk.getData(), 1, chunk.getChunkSize(), m_pimpl->fin);
            if (read < chunk.getChunkSize()) {
                if (std::ferror(m_pimpl->fin) != 0) {
                    GHULBUS_THROW(Ghulbus::Exceptions::IOError{} << Ghulbus::Exception_Info::filename(m_pimpl->filepath.string()),
                                  "Error reading from file.");
                } else if (std::feof(m_pimpl->fin) != 0) {
                    std::fclose(m_pimpl->fin);
                    m_pimpl->fin = nullptr;
                    m_pimpl->filepath = boost::filesystem::path{};
                    do_continue = false;
                } else {
                    GHULBUS_THROW(Ghulbus::Exceptions::IOError{} << Ghulbus::Exception_Info::filename(m_pimpl->filepath.string()),
                                  "Unexpected I/O error.");
                }
            }
            chunk.setUsedSize(read);
            return std::make_tuple(index, do_continue);
    } };
    m_pimpl->outstanding_reads.emplace_back(pt.get_future());
    m_pimpl->pool.schedule([pt = std::move(pt)]() mutable { pt(); });
}

void FileIO::cancelReading()
{
    m_pimpl->pool.cancelAndFlush();
    if (m_pimpl->fin) {
        std::fclose(m_pimpl->fin);
        m_pimpl->fin = nullptr;
        m_pimpl->filepath = boost::filesystem::path{};
    }
    m_pimpl->outstanding_reads.clear();
}

bool FileIO::hasMoreChunks() const
{
    return !m_pimpl->outstanding_reads.empty();
}

FileChunk const& FileIO::getNextChunk()
{
    GHULBUS_PRECONDITION(hasMoreChunks());
    auto const [chunk_index, do_continue] = m_pimpl->outstanding_reads.front().get();
    m_pimpl->outstanding_reads.pop_front();
    if (do_continue) {
        scheduleChunkRead();
    } else {
        for (auto& f : m_pimpl->outstanding_reads) {
            auto const [idx, cc] = f.get();
            GHULBUS_ASSERT(idx == std::numeric_limits<std::size_t>::max());
        }
        m_pimpl->outstanding_reads.clear();
    }
    return m_pimpl->chunks[chunk_index];
}
