// generated by ../external/sqlpp11/scripts/ddl2cpp -fail-on-parse plugin_kv_store.ddl plugin_kv_store blimpdb
#ifndef BLIMPDB_PLUGIN_KV_STORE_H
#define BLIMPDB_PLUGIN_KV_STORE_H

#include <sqlpp11/table.h>
#include <sqlpp11/data_types.h>
#include <sqlpp11/char_sequence.h>

namespace blimpdb
{
namespace PluginKvStore_
{
struct StoreKey
{
    struct _alias_t
    {
        static constexpr const char _literal[] =  "store_key";
        using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
        template<typename T>
        struct _member_t
        {
            T storeKey;
            T& operator()() { return storeKey; }
            const T& operator()() const { return storeKey; }
        };
    };
    using _traits = sqlpp::make_traits<sqlpp::text, sqlpp::tag::can_be_null>;
};
struct Value
{
    struct _alias_t
    {
        static constexpr const char _literal[] =  "value";
        using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
        template<typename T>
        struct _member_t
        {
            T value;
            T& operator()() { return value; }
            const T& operator()() const { return value; }
        };
    };
    using _traits = sqlpp::make_traits<sqlpp::blob, sqlpp::tag::can_be_null>;
};
} // namespace PluginKvStore_

struct PluginKvStore: sqlpp::table_t<PluginKvStore,
    PluginKvStore_::StoreKey,
    PluginKvStore_::Value>
{
    struct _alias_t
    {
        static constexpr const char _literal[] =  "plugin_kv_store";
        using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
        template<typename T>
        struct _member_t
        {
            T pluginKvStore;
            T& operator()() { return pluginKvStore; }
            const T& operator()() const { return pluginKvStore; }
        };
    };
};
} // namespace blimpdb
#endif
