#include <db/blimpdb.hpp>
#include <db/table/blimp_properties.hpp>
#include <db/table/sqlite_master.hpp>

#include <exceptions.hpp>
#include <version.hpp>

#include <gbBase/Assert.hpp>
#include <gbBase/Log.hpp>

#include <gsl.h>

#include <sqlpp11/sqlpp11.h>
#include <sqlpp11/sqlite3/sqlite3.h>

#include <boost/variant.hpp>

#include <cstdint>
#include <limits>
#include <ostream>

void createBlimpPropertiesTable(sqlpp::sqlite3::connection& db)
{
    auto const prop_tab = blimpdb::BlimpProperties{};
    db.execute("CREATE TABLE IF NOT EXISTS blimp_properties(id TEXT PRIMARY KEY, value TEXT);");
    db(sqlpp::insert_into(prop_tab).set(prop_tab.id    = "version",
                                        prop_tab.value = std::to_string(BlimpVersion::version())));
}


/* static */
void BlimpDB::createNewFileDatabase(std::string const& db_filename, std::vector<std::string> const& initialFileSet)
{
    GHULBUS_LOG(Info, "Creating new database at " << db_filename << " from initial set of "
                      << initialFileSet.size() << " element(s).");

    sqlpp::sqlite3::connection_config conf;
    conf.debug = true;
    conf.flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;
    conf.path_to_database = db_filename;

    try {
    sqlpp::sqlite3::connection db(conf);
    auto const master_tab = blimpdb::SqliteMaster{};
    if(db(sqlpp::select(master_tab.name).from(master_tab)
            .where((master_tab.name == "blimp_properties") && (master_tab.type == "table"))).empty())
    {
        createBlimpPropertiesTable(db);
    }

    auto const prop_tab = blimpdb::BlimpProperties{};
    for(auto const& r : db(sqlpp::select(prop_tab.value).from(prop_tab).where(prop_tab.id == "version")))
    {
        GHULBUS_LOG(Info, "ROW: " << r.value);
    }
    } catch(sqlpp::exception& e) {
        GHULBUS_LOG(Error, "Sqlpp exception: " << e.what());
    }

    GHULBUS_LOG(Info, " Successfully established database at " << db_filename << ".");
}

