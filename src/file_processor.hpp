#ifndef BLIMP_INCLUDE_GUARD_FILE_PROCESSOR_HPP
#define BLIMP_INCLUDE_GUARD_FILE_PROCESSOR_HPP

#include <file_info.hpp>

#include <memory>
#include <string>
#include <vector>

struct ProcessedElement
{
    std::string identifier;
    std::vector<char> bytes;
    std::vector<FileInfo> contained_files;
};

class FileProcessor
{
public:
    FileProcessor(std::vector<FileInfo> const& files);
    ~FileProcessor();
    void startProcessing();
    void stopProcessing();
    ProcessedElement getProcessedElement();
    std::size_t getRemainingElements() const;
private:
    void process();
private:
    struct Pimpl;
    std::unique_ptr<Pimpl> m_pimpl;
};

#endif
