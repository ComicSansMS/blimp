#include <processing_pipeline.hpp>

#include <file_chunk.hpp>
#include <file_hash.hpp>
#include <storage_location.hpp>

#include <plugin_common.hpp>
#include <blimp_plugin_sdk.h>

#include <gbBase/Assert.hpp>
#include <gbBase/Log.hpp>

#include <boost/filesystem.hpp>

namespace {
    boost::filesystem::path g_basePath = "./test_storage";
    constexpr int64_t g_sizeLimit = (64 << 20);
}

std::string outPath()
{
    std::size_t i = 0;
    boost::filesystem::path p;
    do {
        p = g_basePath / (std::string{ "s" } + std::to_string(i));
        ++i;
    } while (boost::filesystem::exists(p));
    return p.generic_string();
}

ProcessingPipeline::ProcessingPipeline(BlimpDB& blimpdb)
    :m_startOffset(0), m_sizeCounter(0), m_partCounter(0),
     m_compression(blimpdb, "compression_zlib"), m_encryption(blimpdb, "encryption_aes")
{
    if (!boost::filesystem::is_directory(g_basePath)) {
        boost::filesystem::create_directory(g_basePath);
    }

    m_encryption.setPassword("batteryhorsestaples");
}

ProcessingPipeline::TransactionGuard ProcessingPipeline::startNewContentTransaction(Hash const& data_hash)
{
    m_locations.clear();
    m_partCounter = 0;
    m_startOffset += m_sizeCounter;
    m_sizeCounter = 0;
    if (!m_fout || (!m_fout.is_open())) {
        auto const p = outPath();
        m_fout = std::ofstream(p, std::ios_base::binary);
        m_current_file = p;
        m_encryption.newStorageContainer(BlimpDB::StorageContainerId{ .i = 42 });
    }
    return TransactionGuard(this);
}

void ProcessingPipeline::addFileChunk(FileChunk const& chunk)
{
    m_sizeCounter += chunk.getUsedSize();
    BlimpFileChunk blimp_chunk{ .data = chunk.getData(), .size = static_cast<int64_t>(chunk.getUsedSize()) };
    m_compression.compressFileChunk(blimp_chunk);

    for (;;) {
        BlimpFileChunk const chunk_compressed = m_compression.getCompressedChunk();
        if (!chunk_compressed.data) { break; }
        m_encryption.encryptFileChunk(chunk_compressed);
        for (;;) {
            BlimpFileChunk const chunk_encrypted = m_encryption.getProcessedChunk();
            if (!chunk_encrypted.data) { break; }
            m_fout.write(chunk_encrypted.data, chunk_encrypted.size);
        }
    }

    //m_fout.write(chunk.getData(), chunk.getUsedSize());
    if (m_fout.tellp() > g_sizeLimit) {
        m_encryption.encryptFileChunk(BlimpFileChunk{ .data = nullptr, .size = 0 });
        for (;;) {
            BlimpFileChunk const chunk_encrypted = m_encryption.getProcessedChunk();
            if (!chunk_encrypted.data) { break; }
            m_fout.write(chunk_encrypted.data, chunk_encrypted.size);
        }
        m_locations.push_back(StorageLocation{ .location = m_current_file,
                                               .offset = m_startOffset,
                                               .size = m_sizeCounter,
                                               .part_number = m_partCounter });
        auto const p = outPath();
        m_fout = std::ofstream(p, std::ios_base::binary);
        m_current_file = p;
        m_startOffset = 0;
        m_sizeCounter = 0;
        ++m_partCounter;
    }
}

std::vector<StorageLocation> ProcessingPipeline::commitTransaction(TransactionGuard&& tg)
{
    BlimpFileChunk blimp_chunk{ .data = nullptr, .size = 0 };
    m_compression.compressFileChunk(blimp_chunk);
    for (;;) {
        BlimpFileChunk const chunk_compressed = m_compression.getCompressedChunk();
        if (!chunk_compressed.data) { break; }
        m_encryption.encryptFileChunk(chunk_compressed);
        for (;;) {
            BlimpFileChunk const chunk_encrypted = m_encryption.getProcessedChunk();
            if (!chunk_encrypted.data) { break; }
            m_fout.write(chunk_encrypted.data, chunk_encrypted.size);
        }
    }

    tg.m_requiresAbort = false;
    m_locations.push_back(StorageLocation{ .location = m_current_file,
                                           .offset = m_startOffset,
                                           .size = m_sizeCounter,
                                           .part_number = m_partCounter });
    return m_locations;
}

void ProcessingPipeline::abortTransaction(TransactionGuard&& tg)
{
    tg.m_requiresAbort = false;
}
