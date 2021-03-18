#ifndef BLIMP_INCLUDE_GUARD_STORAGE_PLUGIN_HPP
#define BLIMP_INCLUDE_GUARD_STORAGE_PLUGIN_HPP

#include <boost/filesystem/path.hpp>

#include <memory>

class StoragePlugin {
    struct Pimpl;
    std::unique_ptr<Pimpl> m_pimpl;
public:
    StoragePlugin(boost::filesystem::path const& dll_file);
    ~StoragePlugin();

    StoragePlugin(StoragePlugin const&) = delete;
    StoragePlugin& operator=(StoragePlugin const&) = delete;

    StoragePlugin(StoragePlugin&&);
    StoragePlugin& operator=(StoragePlugin&&);

    char const* plugin_name() const;
};

#endif
