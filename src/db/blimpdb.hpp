#ifndef BLIMP_INCLUDE_GUARD_DB_BLIMPDB_HPP
#define BLIMP_INCLUDE_GUARD_DB_BLIMPDB_HPP

#include <memory>
#include <string>
#include <vector>

class BlimpDB
{
public:
    enum class OpenMode {
        OpenExisting,
        CreateNew
    };
private:
    struct Pimpl;
    std::unique_ptr<Pimpl> m_pimpl;
public:
    BlimpDB(std::string const& db_filename, OpenMode mode);
    ~BlimpDB();
    BlimpDB(BlimpDB const&) = delete;
    BlimpDB& operator=(BlimpDB const&) = delete;

    void setUserSelection(std::vector<std::string> const& selected_files);
private:
    void createNewFileDatabase(std::string const& db_filename);
    void openExistingFileDatabase(std::string const& db_filename);
};

#endif
