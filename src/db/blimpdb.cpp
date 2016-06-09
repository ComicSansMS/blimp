#include <db/blimpdb.hpp>

#include <gbBase/Assert.hpp>
#include <gbBase/Log.hpp>

#include <gsl.h>


/* static */
void BlimpDB::createNewFileDatabase(std::string const& db_filename, std::vector<std::string> const& initialFileSet)
{
    GHULBUS_LOG(Info, "Creating new database at " << db_filename << " from initial set of "
                      << initialFileSet.size() << " element(s).");
    sqlite3* db = nullptr;
    auto db_guard = gsl::finally([&db]() { int const res = sqlite3_close(db); GHULBUS_ASSERT(res == SQLITE_OK); });
    int res = sqlite3_open_v2(db_filename.c_str(), &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr);
    if(res != SQLITE_OK) {
        GHULBUS_LOG(Error, "Error opening sqlite db " << res << ".");
        return;
    }

    char const blimp_properties_table[] = "CREATE TABLE IF NOT EXISTS blimp_properties(key TEXT, value TEXT);";
    sqlite3_stmt* stmt = nullptr;
    char const* tail = nullptr;
    auto stmt_guard = gsl::finally([&stmt]() { sqlite3_finalize(stmt); });
    res = sqlite3_prepare_v2(db, blimp_properties_table, sizeof(blimp_properties_table), &stmt, &tail);
    if(res != SQLITE_OK) {
        GHULBUS_LOG(Error, "Error creating statement for sqlite " << res << ".");
        return;
    }
    res = sqlite3_step(stmt);
    if(res != SQLITE_DONE) {
        GHULBUS_LOG(Error, "Error executing sqlite query " << res << ".");
        return;
    }
    int colcount = sqlite3_column_count(stmt);
    for(int i=0; i<colcount; ++i) {
        int type = sqlite3_column_type(stmt, i);
        GHULBUS_LOG(Trace, i << "st column: " << type);
    }

    GHULBUS_LOG(Info, " Successfully established database at " << db_filename << ".");
}

