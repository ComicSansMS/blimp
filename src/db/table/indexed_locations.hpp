#ifndef BLIMP_INCLUDE_GUARD_DB_TABLE_INDEXED_LOCATIONS_HPP
#define BLIMP_INCLUDE_GUARD_DB_TABLE_INDEXED_LOCATIONS_HPP

#include <sqlpp11/table.h>
#include <sqlpp11/data_types.h>
#include <sqlpp11/char_sequence.h>

namespace blimpdb
{
  namespace IndexedLocations_
  {
    struct LocationId
    {
      struct _alias_t
      {
        static constexpr const char _literal[] =  "location_id";
        using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
        template<typename T>
        struct _member_t
          {
            T locationId;
            T& operator()() { return locationId; }
            const T& operator()() const { return locationId; }
          };
      };
      using _traits = sqlpp::make_traits<sqlpp::integer, sqlpp::tag::must_not_insert, sqlpp::tag::must_not_update>;
    };
    struct Path
    {
      struct _alias_t
      {
        static constexpr const char _literal[] =  "path";
        using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
        template<typename T>
        struct _member_t
          {
            T path;
            T& operator()() { return path; }
            const T& operator()() const { return path; }
          };
      };
      using _traits = sqlpp::make_traits<sqlpp::text, sqlpp::tag::require_insert>;
    };
  }

  struct IndexedLocations: sqlpp::table_t<IndexedLocations,
               IndexedLocations_::LocationId,
               IndexedLocations_::Path>
  {
    struct _alias_t
    {
      static constexpr const char _literal[] =  "indexed_locations";
      using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
      template<typename T>
      struct _member_t
      {
        T indexedLocations;
        T& operator()() { return indexedLocations; }
        const T& operator()() const { return indexedLocations; }
      };
    };
  };
}
#endif
