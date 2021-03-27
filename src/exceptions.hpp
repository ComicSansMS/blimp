#ifndef BLIMP_INCLUDE_GUARD_EXCEPTIONS_HPP
#define BLIMP_INCLUDE_GUARD_EXCEPTIONS_HPP

#include <gbBase/Exception.hpp>

namespace Exception_Info
{
namespace Tags
{
struct sqlite_error_code { };
struct sqlite_query_string { };
struct plugin_name {};
}

namespace Records
{
using plugin_name = Ghulbus::ErrorInfo<Tags::plugin_name, std::string>;
}
}

namespace Exceptions
{
struct DatabaseError : public Ghulbus::Exceptions::IOError {};
struct PluginError : public Ghulbus::Exceptions::ProtocolViolation {};
}
#endif
