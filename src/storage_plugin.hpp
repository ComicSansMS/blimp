#ifndef BLIMP_INCLUDE_GUARD_STORAGE_PLUGIN_HPP
#define BLIMP_INCLUDE_GUARD_STORAGE_PLUGIN_HPP

#include <memory>
#include <string>

class StoragePlugin {
    struct Pimpl;
    std::unique_ptr<Pimpl> m_pimpl;
public:
    StoragePlugin(std::string const& plugin_name);
    ~StoragePlugin();

    StoragePlugin(StoragePlugin const&) = delete;
    StoragePlugin& operator=(StoragePlugin const&) = delete;

    StoragePlugin(StoragePlugin&&);
    StoragePlugin& operator=(StoragePlugin&&);

    char const* plugin_name() const;
};

#endif
