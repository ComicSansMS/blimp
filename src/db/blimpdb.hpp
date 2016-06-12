#ifndef BLIMP_INCLUDE_GUARD_DB_BLIMPDB_HPP
#define BLIMP_INCLUDE_GUARD_DB_BLIMPDB_HPP

#include <sqlite3.h>

#include <string>
#include <vector>

class BlimpDB
{
public:
    static void createNewFileDatabase(std::string const& db_filename, std::vector<std::string> const& initialFileSet);
};

#endif
