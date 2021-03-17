#ifndef BLIMP_INCLUDE_GUARD_DB_TABLE_STORAGE_HPP
#define BLIMP_INCLUDE_GUARD_DB_TABLE_STORAGE_HPP

#include <sqlpp11/table.h>
#include <sqlpp11/data_types.h>
#include <sqlpp11/char_sequence.h>

namespace blimpdb
{
  namespace Storage_
  {
    struct StorageId
    {
      struct _alias_t
      {
        static constexpr const char _literal[] =  "storage_id";
        using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
        template<typename T>
        struct _member_t
          {
            T storageId;
            T& operator()() { return storageId; }
            const T& operator()() const { return storageId; }
          };
      };
      using _traits = sqlpp::make_traits<sqlpp::integer, sqlpp::tag::must_not_insert, sqlpp::tag::must_not_update>;
    };
    struct ContentId
    {
      struct _alias_t
      {
        static constexpr const char _literal[] =  "content_id";
        using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
        template<typename T>
        struct _member_t
          {
            T contentId;
            T& operator()() { return contentId; }
            const T& operator()() const { return contentId; }
          };
      };
      using _traits = sqlpp::make_traits<sqlpp::integer, sqlpp::tag::require_insert>;
    };
  } // namespace Storage_

  struct Storage: sqlpp::table_t<Storage,
               Storage_::StorageId,
               Storage_::ContentId>
  {
    struct _alias_t
    {
      static constexpr const char _literal[] =  "storage";
      using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
      template<typename T>
      struct _member_t
      {
        T storage;
        T& operator()() { return storage; }
        const T& operator()() const { return storage; }
      };
    };
  };
} // namespace blimpdb
#endif
