#include <processing_pipeline.hpp>

#include <file_chunk.hpp>
#include <file_hash.hpp>
#include <storage_location.hpp>

#include <gbBase/Assert.hpp>
#include <gbBase/Log.hpp>

#include <boost/filesystem.hpp>

namespace {
    boost::filesystem::path g_basePath = "./test_storage";
    constexpr int64_t g_sizeLimit = (5 << 20);
}

std::string outPath()
{
    std::size_t i = 0;
    boost::filesystem::path p;
    do {
        p = g_basePath / (std::string{ "s" } + std::to_string(i));
        ++i;
    } while (boost::filesystem::exists(p));
    return p.string();
}

ProcessingPipeline::ProcessingPipeline()
{
    if (!boost::filesystem::is_directory(g_basePath)) {
        boost::filesystem::create_directory(g_basePath);
    }
}

ProcessingPipeline::TransactionGuard ProcessingPipeline::startNewContentTransaction(Hash const& data_hash)
{
    m_locations.clear();
    m_partCounter = 0;
    if (!m_fout || (!m_fout.is_open())) {
        auto const p = outPath();
        m_fout = std::ofstream(p, std::ios_base::binary);
        m_current_file = p;
    }
    std::string delimiter = ("\n\n" + to_string(data_hash) + "\n");
    m_fout.write(delimiter.c_str(), delimiter.size());
    m_startOffset = m_fout.tellp();
    return TransactionGuard(this);
}

void ProcessingPipeline::addFileChunk(FileChunk const& chunk)
{
    m_fout.write(chunk.getData(), chunk.getUsedSize());
    if (m_fout.tellp() > g_sizeLimit) {
        m_locations.push_back(StorageLocation{ .location = m_current_file,
                                               .offset = m_startOffset,
                                               .size = m_fout.tellp() - m_startOffset,
                                               .part_number = m_partCounter });
        auto const p = outPath();
        m_fout = std::ofstream(p, std::ios_base::binary);
        m_current_file = p;
        m_startOffset = 0;
        ++m_partCounter;
    }
}

std::vector<StorageLocation> ProcessingPipeline::commitTransaction(TransactionGuard&& tg)
{
    tg.m_requiresAbort = false;
    m_locations.push_back(StorageLocation{ .location = m_current_file,
                                           .offset = m_startOffset,
                                           .size = m_fout.tellp() - m_startOffset,
                                           .part_number = m_partCounter });
    return m_locations;
}

void ProcessingPipeline::abortTransaction(TransactionGuard&& tg)
{
    tg.m_requiresAbort = false;
}
