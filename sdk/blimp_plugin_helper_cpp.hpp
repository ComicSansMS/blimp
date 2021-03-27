#ifndef BLIMP_INCLUDE_GUARD_BLIMP_PLUGIN_SDK_PLUGIN_HELPER_CPP_HPP
#define BLIMP_INCLUDE_GUARD_BLIMP_PLUGIN_SDK_PLUGIN_HELPER_CPP_HPP

class KeyValueStore {
private:
    BlimpKeyValueStore m_kv;
public:
    explicit KeyValueStore(BlimpKeyValueStore const& kv)
        :m_kv(kv)
    {}

    void store(char const* key, BlimpKeyValueStoreValue v) {
        m_kv.store(m_kv.state, key, v);
    }

    BlimpKeyValueStoreValue retrieve(char const* key) {
        return m_kv.retrieve(m_kv.state, key);
    }
};

#endif
