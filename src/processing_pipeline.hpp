#ifndef BLIMP_INCLUDE_GUARD_PROCESSING_PIPELINE_HPP
#define BLIMP_INCLUDE_GUARD_PROCESSING_PIPELINE_HPP

#include <gbBase/Assert.hpp>

#include <blimp_plugin_sdk.h>

#include <boost/dll/shared_library.hpp>

#include <fstream>
#include <vector>

class FileChunk;
struct Hash;
struct StorageLocation;

class ProcessingPipeline {
public:
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

        void addFileChunk(FileChunk const& chunk)
        {
            GHULBUS_PRECONDITION(m_requiresAbort);
            m_parent->addFileChunk(chunk);
        }
    };
private:
    std::string m_current_file;
    std::ofstream m_fout;
    std::vector<StorageLocation> m_locations;
    std::int64_t m_startOffset;
    std::int64_t m_partCounter;

    boost::dll::shared_library m_compression_dll;
    blimp_plugin_compression_initialize_type m_compression_plugin_initialize;
    blimp_plugin_compression_shutdown_type m_compression_plugin_shutdown;
    BlimpPluginCompression* m_compression;
public:
    ProcessingPipeline();

    ~ProcessingPipeline();

    ProcessingPipeline(ProcessingPipeline const&) = delete;
    ProcessingPipeline& operator=(ProcessingPipeline const&) = delete;

    TransactionGuard startNewContentTransaction(Hash const& data_hash);

    [[nodiscard]] std::vector<StorageLocation> commitTransaction(TransactionGuard&& tg);

    void abortTransaction(TransactionGuard&& tg);

private:
    void addFileChunk(FileChunk const& chunk);
};

#endif
