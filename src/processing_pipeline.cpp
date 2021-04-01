#include <processing_pipeline.hpp>

#include <file_chunk.hpp>
#include <file_hash.hpp>
#include <storage_container.hpp>
#include <storage_location.hpp>

#include <plugin_common.hpp>
#include <plugin_compression.hpp>
#include <plugin_encryption.hpp>
#include <plugin_storage.hpp>

#include <blimp_plugin_sdk.h>

#include <gbBase/Assert.hpp>
#include <gbBase/AnyInvocable.hpp>
#include <gbBase/Log.hpp>

#include <chrono>

namespace {
constexpr std::size_t g_containerSizeLimit = (100 << 20);
}

class PipelineStage {
private:
    PipelineStage* m_downstream;
    Ghulbus::AnyInvocable<void(BlimpFileChunk)> m_funcProcess;
    Ghulbus::AnyInvocable<BlimpFileChunk()> m_funcGetChunk;
    std::size_t m_byteCounter;
    std::size_t m_byteCounterCurrentContainer;
    std::chrono::steady_clock::duration m_timeLastPump;
    std::chrono::steady_clock::duration m_timeTotal;
    std::chrono::steady_clock::duration m_timeTotalCurrentContainer;
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
    std::size_t getByteCounterCurrentContainer() const;
    std::chrono::milliseconds getTimeTotal() const;
    std::chrono::milliseconds getTimeTotalCurrentContainer() const;
    double getBandwidthMbps() const;
    void resetStats();
    void resetStatsCurrentContainer();
private:
    void process(BlimpFileChunk chunk);
    BlimpFileChunk getProcessedChunk();
};

PipelineStage::PipelineStage(Ghulbus::AnyInvocable<void(BlimpFileChunk)> process_func,
                             Ghulbus::AnyInvocable<BlimpFileChunk()> get_func)
    :m_downstream(nullptr), m_funcProcess(std::move(process_func)), m_funcGetChunk(std::move(get_func)),
     m_byteCounter(0), m_byteCounterCurrentContainer(0),
     m_timeLastPump(std::chrono::steady_clock::duration::zero()),
     m_timeTotal(std::chrono::steady_clock::duration::zero()),
     m_timeTotalCurrentContainer(std::chrono::steady_clock::duration::zero())
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
    m_byteCounterCurrentContainer += chunk.size;
    auto const t0 = std::chrono::steady_clock::now();
    process(chunk);
    auto const t1 = std::chrono::steady_clock::now();
    auto const dt = t1 - t0;
    m_timeLastPump = dt;
    m_timeTotal += dt;
    m_timeTotalCurrentContainer += dt;
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

std::size_t PipelineStage::getByteCounterCurrentContainer() const
{
    return m_byteCounterCurrentContainer;
}

std::chrono::milliseconds PipelineStage::getTimeTotal() const
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(m_timeTotal);
}

std::chrono::milliseconds PipelineStage::getTimeTotalCurrentContainer() const
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(m_timeTotalCurrentContainer);
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
    m_byteCounterCurrentContainer = 0;
    m_timeLastPump = std::chrono::steady_clock::duration::zero();
    m_timeTotal = std::chrono::steady_clock::duration::zero();
    m_timeTotalCurrentContainer = std::chrono::steady_clock::duration::zero();
}

void PipelineStage::resetStatsCurrentContainer()
{
    m_byteCounterCurrentContainer = 0;
    m_timeTotalCurrentContainer = std::chrono::steady_clock::duration::zero();
}

struct ProcessingPipeline::Pipeline {
    PluginCompression m_compression;
    PluginEncryption m_encryption;
    PluginStorage m_storage;

    std::vector<PipelineStage> m_stages;

    Pipeline(BlimpDB& blimpdb);

    void resetStatsCurrentContainer();
    void flush();
};

ProcessingPipeline::Pipeline::Pipeline(BlimpDB& blimpdb)
    :m_compression(blimpdb, "compression_zlib"), m_encryption(blimpdb, "encryption_aes"),
     m_storage(blimpdb, "storage_filesystem")
{
    m_encryption.setPassword("batteryhorsestaples");
    m_storage.setBaseLocation("./test_storage");

    m_stages.reserve(3);
    m_stages.emplace_back([this](BlimpFileChunk c) { m_compression.compressFileChunk(c); }, [this]() -> BlimpFileChunk { return m_compression.getCompressedChunk(); });
    m_stages.emplace_back([this](BlimpFileChunk c) { m_encryption.encryptFileChunk(c); }, [this]() -> BlimpFileChunk { return m_encryption.getProcessedChunk(); });
    m_stages.emplace_back([this](BlimpFileChunk c) { m_storage.storeFileChunk(c); }, []() -> BlimpFileChunk { return {}; });
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

void ProcessingPipeline::Pipeline::resetStatsCurrentContainer()
{
    for (auto& s : m_stages) {
        s.resetStatsCurrentContainer();
    }
}

ProcessingPipeline::ProcessingPipeline(BlimpDB& blimpdb)
    :m_startOffset(0), m_sizeCounter(0), m_partCounter(0),
     m_pipeline(std::make_unique<Pipeline>(blimpdb)), m_currentContainerFull(true), m_currentContainerId{ .i = 0 }
{
}

ProcessingPipeline::~ProcessingPipeline() = default;

void ProcessingPipeline::newStorageContainer(StorageContainerId const& container_id)
{
    m_pipeline->m_storage.newStorageContainer(container_id);
    m_pipeline->m_encryption.newStorageContainer(container_id);
    m_pipeline->resetStatsCurrentContainer();

    m_startOffset = 0;
    m_sizeCounter = 0;
    m_currentContainerFull = false;
    m_currentContainerId = container_id;
}

ProcessingPipeline::TransactionGuard ProcessingPipeline::startNewContentTransaction(Hash const& data_hash)
{
    m_locations.clear();
    m_partCounter = 0;
    m_startOffset += m_sizeCounter;
    m_sizeCounter = 0;

    return TransactionGuard(this);
}

ProcessingPipeline::ContainerStatus ProcessingPipeline::addFileChunk(FileChunk const& chunk)
{
    if (m_currentContainerFull) { return ContainerStatus::Full; }
    m_sizeCounter += chunk.getUsedSize();
    BlimpFileChunk blimp_chunk{ .data = chunk.getData(), .size = static_cast<int64_t>(chunk.getUsedSize()) };

    m_pipeline->m_stages.front().pump(blimp_chunk);
    if (m_pipeline->m_stages.back().getByteCounterCurrentContainer() > g_containerSizeLimit) {
        m_pipeline->flush();
        m_locations.push_back(StorageLocation{ .container_id = m_currentContainerId,
                                               .offset = m_startOffset,
                                               .size = m_sizeCounter,
                                               .part_number = m_partCounter });
        BlimpStorageContainerLocation container_location = m_pipeline->m_storage.finalizeStorageContainer();
        m_lastContainerLocation = StorageContainerLocation{ .l = container_location.location };
        ++m_partCounter;
        m_currentContainerFull = true;
        m_currentContainerId = StorageContainerId{ .i = 0 };
        return ContainerStatus::Full;
    }
    return ContainerStatus::Ok;
}

std::vector<StorageLocation> ProcessingPipeline::commitTransaction(TransactionGuard&& tg)
{
    // flush compression
    m_pipeline->m_stages.front().flushStage();
    tg.m_requiresAbort = false;
    m_locations.push_back(StorageLocation{ .container_id = m_currentContainerId,
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
    if (m_currentContainerId.i != 0) {
        m_pipeline->flush();
        BlimpStorageContainerLocation container_location = m_pipeline->m_storage.finalizeStorageContainer();
        m_currentContainerId = StorageContainerId{ 0 };
        m_lastContainerLocation = StorageContainerLocation{ .l = container_location.location };
    }
    GHULBUS_LOG(Debug, "Processing stastics per pipeline stage:");
    for (std::size_t i = 0, i_end = m_pipeline->m_stages.size(); i != i_end; ++i) {
        auto const& s = m_pipeline->m_stages[i];
        GHULBUS_LOG(Debug, "Stage #" << i << ": " << s.getTimeTotal() << " (" << s.getBandwidthMbps() << "Mbps)");
    }
}

StorageContainerLocation ProcessingPipeline::getLastContainerLocation() const
{
    return m_lastContainerLocation;
}
