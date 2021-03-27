#include <plugin_key_value_store.hpp>

#include <db/blimpdb.hpp>

struct BlimpKeyValueStoreState {
    BlimpDB* blimpdb;
    BlimpPluginInfo plugin_info;
    BlimpDB::PluginStoreValue cached_value;
};

PluginKeyValueStore::PluginKeyValueStore(BlimpDB& blimpdb, BlimpPluginInfo const& plugin_info)
    :m_kvState(std::make_unique<BlimpKeyValueStoreState>())
{
    m_kvState->blimpdb = &blimpdb;
    m_kvState->plugin_info = plugin_info;
}

PluginKeyValueStore::~PluginKeyValueStore() = default;

BlimpKeyValueStore PluginKeyValueStore::getPluginKeyValueStore() const
{
    BlimpKeyValueStore ret;
    ret.state = m_kvState.get();
    ret.store = [](BlimpKeyValueStoreStateHandle state, char const* key, BlimpKeyValueStoreValue value) {
        state->blimpdb->pluginStoreValue(state->plugin_info, key, value);
    };
    ret.retrieve = [](BlimpKeyValueStoreStateHandle state, char const* key) -> BlimpKeyValueStoreValue {
        state->cached_value = state->blimpdb->pluginRetrieveValue(state->plugin_info, key);
        return state->cached_value.value;
    };
    return ret;
}
