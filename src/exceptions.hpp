#ifndef BLIMP_INCLUDE_GUARD_EXCEPTIONS_HPP
#define BLIMP_INCLUDE_GUARD_EXCEPTIONS_HPP

#include <gbBase/Exception.hpp>

namespace Exception_Info {
namespace Tags
{
struct sqlite_error_code { };
struct sqlite_query_string { };
}

namespace Records
{
}

typedef boost::error_info<Tags::sqlite_error_code, int> sqlite_error_code;
typedef boost::error_info<Tags::sqlite_query_string, std::string> sqlite_query_string;
}

struct SqliteException : public Ghulbus::Exceptions::IOError {};

#endif
