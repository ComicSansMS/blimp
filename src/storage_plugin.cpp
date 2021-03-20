#include <storage_plugin.hpp>
#include <plugin_common.hpp>

#include <blimp_plugin_sdk.h>

#include <boost/dll.hpp>

#include <functional>

struct StoragePlugin::Pimpl {
    boost::dll::shared_library dll;

    blimp_plugin_api_info_type api_info;

    Pimpl(std::string const& plugin_name)
        :dll(load_plugin_by_name(plugin_name))
    {
        api_info = dll.get<BlimpPluginInfo()>("blimp_plugin_api_info");
    }
};

StoragePlugin::StoragePlugin(std::string const& plugin_name)
    :m_pimpl(std::make_unique<Pimpl>(plugin_name))
{}

StoragePlugin::~StoragePlugin() = default;
StoragePlugin::StoragePlugin(StoragePlugin&&) = default;
StoragePlugin& StoragePlugin::operator=(StoragePlugin&&) = default;

char const* StoragePlugin::plugin_name() const
{
    return m_pimpl->api_info().name;
}
