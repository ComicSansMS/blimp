#include <processing_pipeline.hpp>

#include <file_chunk.hpp>
#include <file_hash.hpp>
#include <storage_location.hpp>

#include <plugin_common.hpp>
#include <blimp_plugin_sdk.h>

#include <gbBase/Assert.hpp>
#include <gbBase/AnyInvocable.hpp>
#include <gbBase/Log.hpp>

#include <boost/filesystem.hpp>

#include <chrono>

namespace {
    boost::filesystem::path g_basePath = "./test_storage";
    constexpr int64_t g_sizeLimit = (64 << 20);
}

class PipelineStage {
private:
    PipelineStage* m_downstream;
    Ghulbus::AnyInvocable<void(BlimpFileChunk)> m_funcProcess;
    Ghulbus::AnyInvocable<BlimpFileChunk()> m_funcGetChunk;
    std::size_t m_byteCounter;
    std::chrono::steady_clock::duration m_timeLastPump;
    std::chrono::steady_clock::duration m_timeTotal;
public:
    PipelineStage(Ghulbus::AnyInvocable<void(BlimpFileChunk)> process_func,
                  Ghulbus::AnyInvocable<BlimpFileChunk()> get_func);
    ~PipelineStage();
    PipelineStage(PipelineStage&&) = default;
    PipelineStage& operator=(PipelineStage&&) = default;
    void setDownstream(PipelineStage& downstream);
    void pump(BlimpFileChunk chunk);
    void flushStage();
    std::size_t getByteCounter() const;
    std::chrono::milliseconds getTimeTotal() const;
    double getBandwidthMbps() const;
    void resetStats();
private:
    void process(BlimpFileChunk chunk);
    BlimpFileChunk getProcessedChunk();
};

PipelineStage::PipelineStage(Ghulbus::AnyInvocable<void(BlimpFileChunk)> process_func,
                             Ghulbus::AnyInvocable<BlimpFileChunk()> get_func)
    :m_downstream(nullptr), m_funcProcess(std::move(process_func)), m_funcGetChunk(std::move(get_func)),
     m_byteCounter(0), m_timeLastPump(std::chrono::steady_clock::duration::zero()),
     m_timeTotal(std::chrono::steady_clock::duration::zero())
{}

PipelineStage::~PipelineStage()
{
    GHULBUS_ASSERT(m_funcGetChunk.empty() || (getProcessedChunk().data == nullptr));
}

void PipelineStage::setDownstream(PipelineStage& downstream)
{
    m_downstream = &downstream;
}

void PipelineStage::pump(BlimpFileChunk chunk)
{
    m_byteCounter += chunk.size;
    auto const t0 = std::chrono::steady_clock::now();
    process(chunk);
    auto const t1 = std::chrono::steady_clock::now();
    auto const dt = t1 - t0;
    m_timeLastPump = dt;
    m_timeTotal += dt;
    if (m_downstream) {
        for(BlimpFileChunk c = getProcessedChunk(); c.data != nullptr; c = getProcessedChunk()) {
            m_downstream->pump(c);
        }
    }
}

void PipelineStage::flushStage()
{
    pump(BlimpFileChunk{ .data = nullptr, .size = 0 });
}

void PipelineStage::process(BlimpFileChunk chunk)
{
    m_funcProcess(chunk);
}

BlimpFileChunk PipelineStage::getProcessedChunk()
{
    return m_funcGetChunk();
}

std::size_t PipelineStage::getByteCounter() const
{
    return m_byteCounter;
}

std::chrono::milliseconds PipelineStage::getTimeTotal() const
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(m_timeTotal);
}

double PipelineStage::getBandwidthMbps() const
{
    auto const mbytes_processed = static_cast<double>(m_byteCounter) / (1024.0 * 1024.0);
    auto const seconds_spent = static_cast<double>(getTimeTotal().count()) / 1000.0;
    return mbytes_processed / seconds_spent;
}

void PipelineStage::resetStats()
{
    m_byteCounter = 0;
    m_timeLastPump = std::chrono::steady_clock::duration::zero();
    m_timeTotal = std::chrono::steady_clock::duration::zero();
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

struct ProcessingPipeline::Pipeline {
    PluginCompression m_compression;
    PluginEncryption m_encryption;
    std::ofstream m_fout;

    std::vector<PipelineStage> m_stages;

    Pipeline(BlimpDB& blimpdb);

    void flush();
};

ProcessingPipeline::Pipeline::Pipeline(BlimpDB& blimpdb)
    :m_compression(blimpdb, "compression_zlib"), m_encryption(blimpdb, "encryption_aes")
{
    m_encryption.setPassword("batteryhorsestaples");

    m_stages.reserve(3);
    m_stages.emplace_back([this](BlimpFileChunk c) { m_compression.compressFileChunk(c); }, [this]() -> BlimpFileChunk { return m_compression.getCompressedChunk(); });
    m_stages.emplace_back([this](BlimpFileChunk c) { m_encryption.encryptFileChunk(c); }, [this]() -> BlimpFileChunk { return m_encryption.getProcessedChunk(); });
    m_stages.emplace_back([this](BlimpFileChunk c) { if (c.data) { m_fout.write(c.data, c.size); } }, [this]() -> BlimpFileChunk { return {}; });
    for (std::size_t i = 0, i_end = m_stages.size() - 1; i != i_end; ++i) {
        m_stages[i].setDownstream(m_stages[i+1]);
    }
}

void ProcessingPipeline::Pipeline::flush()
{
    for (auto& s : m_stages) {
        s.pump(BlimpFileChunk{ .data = nullptr, .size = 0 });
    }
}

ProcessingPipeline::ProcessingPipeline(BlimpDB& blimpdb)
    :m_startOffset(0), m_sizeCounter(0), m_partCounter(0),
     m_pipeline(std::make_unique<Pipeline>(blimpdb))
{
    if (!boost::filesystem::is_directory(g_basePath)) {
        boost::filesystem::create_directory(g_basePath);
    }
}

ProcessingPipeline::~ProcessingPipeline() = default;

ProcessingPipeline::TransactionGuard ProcessingPipeline::startNewContentTransaction(Hash const& data_hash)
{
    m_locations.clear();
    m_partCounter = 0;
    m_startOffset += m_sizeCounter;
    m_sizeCounter = 0;
    if (!m_pipeline->m_fout || (!m_pipeline->m_fout.is_open())) {
        auto const p = outPath();
        m_pipeline->m_fout = std::ofstream(p, std::ios_base::binary);
        m_current_file = p;
        m_pipeline->m_encryption.newStorageContainer(BlimpDB::StorageContainerId{ .i = 42 });
    }
    return TransactionGuard(this);
}

void ProcessingPipeline::addFileChunk(FileChunk const& chunk)
{
    m_sizeCounter += chunk.getUsedSize();
    BlimpFileChunk blimp_chunk{ .data = chunk.getData(), .size = static_cast<int64_t>(chunk.getUsedSize()) };

    m_pipeline->m_stages.front().pump(blimp_chunk);
    if (m_pipeline->m_stages.back().getByteCounter() > g_sizeLimit) {
        m_pipeline->flush();
        m_locations.push_back(StorageLocation{ .location = m_current_file,
                                               .offset = m_startOffset,
                                               .size = m_sizeCounter,
                                               .part_number = m_partCounter });
        auto const p = outPath();
        m_pipeline->m_fout = std::ofstream(p, std::ios_base::binary);
        m_current_file = p;
        m_startOffset = 0;
        m_sizeCounter = 0;
        ++m_partCounter;
    }
}

std::vector<StorageLocation> ProcessingPipeline::commitTransaction(TransactionGuard&& tg)
{
    // flush compression
    m_pipeline->m_stages.front().flushStage();
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

void ProcessingPipeline::finish()
{
    m_pipeline->flush();
    GHULBUS_LOG(Debug, "Processing stastics per pipeline stage:");
    for (std::size_t i = 0, i_end = m_pipeline->m_stages.size(); i != i_end; ++i) {
        auto const& s = m_pipeline->m_stages[i];
        GHULBUS_LOG(Debug, "Stage #" << i << ": " << s.getTimeTotal() << " (" << s.getBandwidthMbps() << "Mbps)");
    }
}
