#ifndef BLIMP_INCLUDE_GUARD_PLUGIN_COMMON_HPP
#define BLIMP_INCLUDE_GUARD_PLUGIN_COMMON_HPP

#include <boost/dll/shared_library.hpp>

#include <string>

boost::dll::shared_library load_plugin_by_name(std::string const& plugin_name);

#endif
