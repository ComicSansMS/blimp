#include <storage_plugin.hpp>

#include <blimp_plugin_sdk.h>

#include <boost/dll.hpp>

#include <functional>

struct StoragePlugin::Pimpl {
    boost::dll::shared_library dll;

    blimp_plugin_api_info_type api_info;

    Pimpl(boost::filesystem::path const& dll_file)
        :dll(dll_file)
    {
        api_info = dll.get<BlimpPluginInfo()>("blimp_plugin_api_info");
    }
};

StoragePlugin::StoragePlugin(boost::filesystem::path const& dll_file)
    :m_pimpl(std::make_unique<Pimpl>(dll_file))
{}

StoragePlugin::~StoragePlugin() = default;
StoragePlugin::StoragePlugin(StoragePlugin&&) = default;
StoragePlugin& StoragePlugin::operator=(StoragePlugin&&) = default;

char const* StoragePlugin::plugin_name() const
{
    return m_pimpl->api_info().name;
}
