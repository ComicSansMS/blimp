#ifndef BLIMP_INCLUDE_GUARD_DB_TABLE_BLIMP_PROPERTIES_HPP
#define BLIMP_INCLUDE_GUARD_DB_TABLE_BLIMP_PROPERTIES_HPP

#include <sqlpp11/table.h>
#include <sqlpp11/data_types.h>
#include <sqlpp11/char_sequence.h>

namespace blimpdb
{
namespace BlimpProperties_
{
struct Id
{
    struct _alias_t
    {
        static constexpr const char _literal[] =  "id";
        using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
        template<typename T>
        struct _member_t
        {
            T id;
            T& operator()() { return id; }
            const T& operator()() const { return id; }
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
    using _traits = sqlpp::make_traits<sqlpp::text, sqlpp::tag::can_be_null>;
};
}

struct BlimpProperties: sqlpp::table_t<BlimpProperties,
    BlimpProperties_::Id,
    BlimpProperties_::Value>
{
    struct _alias_t
    {
        static constexpr const char _literal[] =  "blimp_properties";
        using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
        template<typename T>
        struct _member_t
        {
            T blimpProperties;
            T& operator()() { return blimpProperties; }
            const T& operator()() const { return blimpProperties; }
        };
    };
};
}

#endif
