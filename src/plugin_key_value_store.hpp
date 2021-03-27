#ifndef BLIMP_INCLUDE_GUARD_PLUGIN_KEY_VALUE_STORE_HPP
#define BLIMP_INCLUDE_GUARD_PLUGIN_KEY_VALUE_STORE_HPP

#include <blimp_plugin_sdk.h>

#include <memory>

class BlimpDB;

class PluginKeyValueStore {
private:
    std::unique_ptr<BlimpKeyValueStoreState> m_kvState;
public:
    explicit PluginKeyValueStore(BlimpDB& blimpdb, BlimpPluginInfo const& plugin_info);
    ~PluginKeyValueStore();

    BlimpKeyValueStore getPluginKeyValueStore() const;
};

#endif
