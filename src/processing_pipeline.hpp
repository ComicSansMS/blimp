#ifndef BLIMP_INCLUDE_GUARD_PROCESSING_PIPELINE_HPP
#define BLIMP_INCLUDE_GUARD_PROCESSING_PIPELINE_HPP

#include <storage_container.hpp>

#include <gbBase/Assert.hpp>

#include <memory>
#include <vector>

class BlimpDB;
class FileChunk;
struct Hash;
struct StorageLocation;

class ProcessingPipeline {
public:
    enum class [[nodiscard]] ContainerStatus {
        Ok,
        Full
    };

    class [[nodiscard]] TransactionGuard {
        friend class ProcessingPipeline;
    private:
        ProcessingPipeline* m_parent;
        bool m_requiresAbort;
    private:
        TransactionGuard(ProcessingPipeline* parent)
            :m_parent(parent), m_requiresAbort(true)
        {}
    public:
        TransactionGuard(TransactionGuard&& rhs)
            :m_parent(rhs.m_parent), m_requiresAbort(rhs.m_requiresAbort)
        {
            rhs.m_parent = nullptr;
            rhs.m_requiresAbort = false;
        }

        TransactionGuard& operator=(TransactionGuard&& rhs)
        {
            if (this != &rhs) {
                if (m_requiresAbort) { m_parent->abortTransaction(std::move(*this)); }
                m_parent = rhs.m_parent;
                rhs.m_parent = nullptr;
                m_requiresAbort = rhs.m_requiresAbort;
                rhs.m_requiresAbort = false;
            }
            return *this;
        }

        ~TransactionGuard()
        {
            if (m_requiresAbort) {
                m_parent->abortTransaction(std::move(*this));
            }
        }

        ContainerStatus addFileChunk(FileChunk const& chunk)
        {
            GHULBUS_PRECONDITION(m_requiresAbort);
            return m_parent->addFileChunk(chunk);
        }
    };
private:
    std::vector<StorageLocation> m_locations;
    std::int64_t m_startOffset;
    std::int64_t m_sizeCounter;
    std::int64_t m_partCounter;
    bool m_currentContainerFull;
    StorageContainerId m_currentContainerId;
    StorageContainerLocation m_lastContainerLocation;

    struct Pipeline;
    std::unique_ptr<Pipeline> m_pipeline;
public:
    explicit ProcessingPipeline(BlimpDB& blimpdb);

    ~ProcessingPipeline();

    ProcessingPipeline(ProcessingPipeline const&) = delete;
    ProcessingPipeline& operator=(ProcessingPipeline const&) = delete;

    void newStorageContainer(StorageContainerId const& container_id);

    TransactionGuard startNewContentTransaction(Hash const& data_hash);

    std::vector<StorageLocation> commitTransaction(TransactionGuard&& tg);

    void abortTransaction(TransactionGuard&& tg);

    void finish();

    StorageContainerLocation getLastContainerLocation() const;

private:
    ContainerStatus addFileChunk(FileChunk const& chunk);
};

#endif
