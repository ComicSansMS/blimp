#ifndef BLIMP_INCLUDE_GUARD_DB_TABLE_STORAGE_CONTAINERS_HPP
#define BLIMP_INCLUDE_GUARD_DB_TABLE_STORAGE_CONTAINERS_HPP

#include <sqlpp11/table.h>
#include <sqlpp11/data_types.h>
#include <sqlpp11/char_sequence.h>

namespace blimpdb
{
  namespace StorageContainers_
  {
    struct ContainerId
    {
      struct _alias_t
      {
        static constexpr const char _literal[] =  "container_id";
        using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
        template<typename T>
        struct _member_t
          {
            T containerId;
            T& operator()() { return containerId; }
            const T& operator()() const { return containerId; }
          };
      };
      using _traits = sqlpp::make_traits<sqlpp::integer, sqlpp::tag::can_be_null>;
    };
    struct Location
    {
      struct _alias_t
      {
        static constexpr const char _literal[] =  "location";
        using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
        template<typename T>
        struct _member_t
          {
            T location;
            T& operator()() { return location; }
            const T& operator()() const { return location; }
          };
      };
      using _traits = sqlpp::make_traits<sqlpp::text, sqlpp::tag::can_be_null>;
    };
  } // namespace StorageContainers_

  struct StorageContainers: sqlpp::table_t<StorageContainers,
               StorageContainers_::ContainerId,
               StorageContainers_::Location>
  {
    struct _alias_t
    {
      static constexpr const char _literal[] =  "storage_containers";
      using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
      template<typename T>
      struct _member_t
      {
        T storageContainers;
        T& operator()() { return storageContainers; }
        const T& operator()() const { return storageContainers; }
      };
    };
  };
} // namespace blimpdb
#endif
