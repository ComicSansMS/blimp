#ifndef BLIMP_INCLUDE_GUARD_EXCEPTIONS_HPP
#define BLIMP_INCLUDE_GUARD_EXCEPTIONS_HPP

#include <gbBase/Exception.hpp>

namespace Exception_Info
{
namespace Tags
{
struct sqlite_error_code { };
struct sqlite_query_string { };
}

namespace Records
{
}
}

namespace Exceptions
{
struct DatabaseError : public Ghulbus::Exceptions::IOError {};
struct PluginError : public Ghulbus::Exceptions::ProtocolViolation {};
}
#endif
