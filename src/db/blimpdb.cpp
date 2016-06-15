#include <db/blimpdb.hpp>

#include <exceptions.hpp>
#include <version.hpp>

#include <gbBase/Assert.hpp>
#include <gbBase/Log.hpp>

#include <gsl.h>

#include <boost/variant.hpp>

#include <cstdint>
#include <limits>
#include <ostream>

enum class ValueType
{
    Null    = SQLITE_NULL,
    Integer = SQLITE_INTEGER,
    Real    = SQLITE_FLOAT,
    Text    = SQLITE_TEXT,
    Blob    = SQLITE_BLOB
};

std::ostream& operator<<(std::ostream& os, ValueType vt)
{
    switch(vt) {
    case ValueType::Null:    return os << "NULL";
    case ValueType::Integer: return os << "INTEGER";
    case ValueType::Real:    return os << "REAL";
    case ValueType::Text:    return os << "TEXT";
    case ValueType::Blob:    return os << "BLOB";
    }
    return os;
}

template<ValueType type>
struct TypeForValueType;

struct NullType {};

template<> struct TypeForValueType<ValueType::Null> {    typedef NullType               type; };
template<> struct TypeForValueType<ValueType::Integer> { typedef int64_t                type; };
template<> struct TypeForValueType<ValueType::Real> {    typedef double                 type; };
template<> struct TypeForValueType<ValueType::Text> {    typedef std::string            type; };
template<> struct TypeForValueType<ValueType::Blob> {    typedef std::vector<gsl::byte> type; };

typedef boost::variant<TypeForValueType<ValueType::Null>::type, TypeForValueType<ValueType::Integer>::type, 
                       TypeForValueType<ValueType::Real>::type, TypeForValueType<ValueType::Text>::type,
                       TypeForValueType<ValueType::Blob>::type> Value;

Value extractColumnValue(sqlite3_stmt* stmt, int iCol)
{
    auto type = static_cast<ValueType>(sqlite3_column_type(stmt, iCol));
    switch(type) {
    default:
    case ValueType::Null:
        return NullType{};
    case ValueType::Integer:
        return sqlite3_column_int64(stmt, iCol);
    case ValueType::Real:
        return sqlite3_column_double(stmt, iCol);
    case ValueType::Text:
        return std::string(reinterpret_cast<char const*>(sqlite3_column_text(stmt, iCol)));
    case ValueType::Blob:
        {
            auto const data = reinterpret_cast<gsl::byte const*>(sqlite3_column_blob(stmt, iCol));
            auto const blob_size = sqlite3_column_bytes(stmt, iCol);
            return std::vector<gsl::byte>(data, data + blob_size);
        }
    }
}

struct ValuePrintVisitor : boost::static_visitor<void>
{
    std::ostream* m_os;

    ValuePrintVisitor(std::ostream& os) : m_os(&os) {}

    void operator()(TypeForValueType<ValueType::Null>::type const&)
    {
        *m_os << "NULL";
    }

    void operator()(TypeForValueType<ValueType::Integer>::type const& i)
    {
        *m_os << i;
    }

    void operator()(TypeForValueType<ValueType::Real>::type const& d)
    {
        *m_os << d;
    }

    void operator()(TypeForValueType<ValueType::Text>::type const& str)
    {
        *m_os << str;
    }

    void operator()(TypeForValueType<ValueType::Blob>::type const& v)
    {
        *m_os << "<Blob of size" << v.size() << ">";
    }
};

std::ostream& operator<<(std::ostream& os, Value const& v)
{
    v.apply_visitor(ValuePrintVisitor(os));
    return os;
}


class Query
{
public:
    enum class State {
        RowRetrieved,
        Completed
    };
private:
    sqlite3_stmt* m_stmt;
public:
    Query(sqlite3* db, std::string const& query_str);
    ~Query();
    Query(Query const&) = delete;
    Query& operator=(Query const&) = delete;

    void bind(std::string const& param, TypeForValueType<ValueType::Integer>::type value);
    void bind(std::string const& param, TypeForValueType<ValueType::Real>::type value);
    void bind(std::string const& param, TypeForValueType<ValueType::Text>::type const& value);
    void bind(std::string const& param, TypeForValueType<ValueType::Blob>::type const& value);
    void bind(std::string const& param, TypeForValueType<ValueType::Null>::type const& value);

    void bind(int param_index, TypeForValueType<ValueType::Integer>::type value);
    void bind(int param_index, TypeForValueType<ValueType::Real>::type value);
    void bind(int param_index, TypeForValueType<ValueType::Text>::type const& value);
    void bind(int param_index, TypeForValueType<ValueType::Blob>::type const& value);
    void bind(int param_index, TypeForValueType<ValueType::Null>::type const& value);

    State step();

    sqlite3_stmt* getStmt() { /* TODO */ return m_stmt; }

private:
    int getParamIndexFromParamString(std::string const& param) const;
};

Query::Query(sqlite3* db, std::string const& query_str)
{
    char const* tail = nullptr;
    GHULBUS_ASSERT_PRD(query_str.size() < std::numeric_limits<int>::max());
    int res = sqlite3_prepare_v2(db, query_str.c_str(), static_cast<int>(query_str.size()) + 1, &m_stmt, &tail);
    if(res != SQLITE_OK)
    {
        GHULBUS_THROW(SqliteException()
                      << Exception_Info::sqlite_error_code(res) << Exception_Info::sqlite_query_string(query_str),
                      "Error creating query from string.");
    }
    if(tail - query_str.c_str() != query_str.size())
    {
        sqlite3_finalize(m_stmt);
        GHULBUS_THROW(SqliteException() << Exception_Info::sqlite_query_string(query_str),
                      "Query string was only partially processed.");
    }
}

Query::~Query()
{
    sqlite3_finalize(m_stmt);
}

int Query::getParamIndexFromParamString(std::string const& param) const
{
    int res = sqlite3_bind_parameter_index(m_stmt, param.c_str());
    if(res == 0) {
        GHULBUS_THROW(Ghulbus::Exceptions::InvalidArgument()
                      << Exception_Info::sqlite_query_string(sqlite3_sql(m_stmt)),
                      "Parameter not found in query.");
    }
    return res;
}

void Query::bind(std::string const& param, TypeForValueType<ValueType::Integer>::type value)
{
    bind(getParamIndexFromParamString(param), value);
}

void Query::bind(std::string const& param, TypeForValueType<ValueType::Real>::type value)
{
    bind(getParamIndexFromParamString(param), value);
}

void Query::bind(std::string const& param, TypeForValueType<ValueType::Text>::type const& value)
{
    bind(getParamIndexFromParamString(param), value);
}

void Query::bind(std::string const& param, TypeForValueType<ValueType::Blob>::type const& value)
{
    bind(getParamIndexFromParamString(param), value);
}

void Query::bind(std::string const& param, TypeForValueType<ValueType::Null>::type const& value)
{
    bind(getParamIndexFromParamString(param), value);
}


void Query::bind(int param_index, TypeForValueType<ValueType::Integer>::type value)
{
    int const res = sqlite3_bind_int64(m_stmt, param_index, value);
    if(res != SQLITE_OK) {
        GHULBUS_THROW(SqliteException() << Exception_Info::sqlite_error_code(res),
                      "Binding Integer argument failed.");
    }
}

void Query::bind(int param_index, TypeForValueType<ValueType::Real>::type value)
{
    int const res = sqlite3_bind_double(m_stmt, param_index, value);
    if(res != SQLITE_OK) {
        GHULBUS_THROW(SqliteException() << Exception_Info::sqlite_error_code(res),
                      "Binding Real argument failed.");
    }
}

void Query::bind(int param_index, TypeForValueType<ValueType::Text>::type const& value)
{
    int const res = sqlite3_bind_text64(m_stmt, param_index, value.c_str(), value.size() + 1,
                                        SQLITE_TRANSIENT, SQLITE_UTF8);
    if(res != SQLITE_OK) {
        GHULBUS_THROW(SqliteException() << Exception_Info::sqlite_error_code(res),
                      "Binding Text argument failed.");
    }
}

void Query::bind(int param_index, TypeForValueType<ValueType::Blob>::type const& value)
{
    int const res = sqlite3_bind_blob64(m_stmt, param_index, value.data(), value.size(), SQLITE_TRANSIENT);
    if(res != SQLITE_OK) {
        GHULBUS_THROW(SqliteException() << Exception_Info::sqlite_error_code(res),
                      "Binding Blob argument failed.");
    }
}

void Query::bind(int param_index, TypeForValueType<ValueType::Null>::type const& value)
{
    int const res = sqlite3_bind_null(m_stmt, param_index);
    if(res != SQLITE_OK) {
        GHULBUS_THROW(SqliteException() << Exception_Info::sqlite_error_code(res),
                      "Binding Null argument failed.");
    }
}

Query::State Query::step()
{
    int const res = sqlite3_step(m_stmt);
    if(res == SQLITE_ROW) {
        return State::RowRetrieved;
    } else if(res == SQLITE_DONE) {
        return State::Completed;
    }
    GHULBUS_THROW(SqliteException() << Exception_Info::sqlite_error_code(res),
                  "Executing query step failed.");
}

void createBlimpPropertiesTable(sqlite3* db)
{
    {
        char const blimp_properties_table[] = "CREATE TABLE IF NOT EXISTS blimp_properties(key TEXT PRIMARY KEY, value TEXT);";
        Query q_table_create(db, blimp_properties_table);
        auto const res = q_table_create.step();
        GHULBUS_ASSERT(res == Query::State::Completed);
    }
    {
        Query q_insert_version(db, "INSERT INTO blimp_properties VALUES (@key, @value);");
        q_insert_version.bind("@key", "version");
        q_insert_version.bind("@value", std::to_string(BlimpVersion::version()));
        auto const res = q_insert_version.step();
        GHULBUS_ASSERT(res == Query::State::Completed);
    }
}


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

    createBlimpPropertiesTable(db);

    //{
    //    Query q(db, "SELECT * FROM sqlite_master WHERE type=@tabtype;");
    //    q.bind("@tabtype", "table");
    //    q.bind("@name", "blimp_properties");
    //    while(q.step() != Query::State::Completed) {
    //        auto stmt = q.getStmt();
    //        int colcount = sqlite3_column_count(stmt);
    //        for(int i=0; i<colcount; ++i) {
    //            auto type = static_cast<ValueType>(sqlite3_column_type(stmt, i));
    //            GHULBUS_LOG(Trace, i << "st column: " << sqlite3_column_name(stmt, i)
    //                << " [" << type << "] - " << extractColumnValue(stmt, i));
    //        }
    //    }
    //}
    Query q(db, "SELECT * FROM blimp_properties WHERE key=@key;");
    q.bind("@key", "version");
    GHULBUS_LOG(Info, sqlite3_sql(q.getStmt()));
    while(q.step() != Query::State::Completed) {
        auto stmt = q.getStmt();
        int colcount = sqlite3_column_count(stmt);
        for(int i=0; i<colcount; ++i) {
            auto type = static_cast<ValueType>(sqlite3_column_type(stmt, i));
            GHULBUS_LOG(Trace, i << "st column: " << sqlite3_column_name(stmt, i)
                                 << " [" << type << "] - " << extractColumnValue(stmt, i));
        }
    }

    GHULBUS_LOG(Info, " Successfully established database at " << db_filename << ".");
}

