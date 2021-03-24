#ifndef BLIMP_INCLUDE_GUARD_DB_TABLE_STORAGE_CONTENTS_HPP
#define BLIMP_INCLUDE_GUARD_DB_TABLE_STORAGE_CONTENTS_HPP

#include <sqlpp11/table.h>
#include <sqlpp11/data_types.h>
#include <sqlpp11/char_sequence.h>

namespace blimpdb
{
  namespace StorageContents_
  {
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
      using _traits = sqlpp::make_traits<sqlpp::text, sqlpp::tag::require_insert>;
    };
    struct Offset
    {
      struct _alias_t
      {
        static constexpr const char _literal[] =  "offset";
        using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
        template<typename T>
        struct _member_t
          {
            T offset;
            T& operator()() { return offset; }
            const T& operator()() const { return offset; }
          };
      };
      using _traits = sqlpp::make_traits<sqlpp::integer, sqlpp::tag::require_insert>;
    };
    struct Size
    {
      struct _alias_t
      {
        static constexpr const char _literal[] =  "size";
        using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
        template<typename T>
        struct _member_t
          {
            T size;
            T& operator()() { return size; }
            const T& operator()() const { return size; }
          };
      };
      using _traits = sqlpp::make_traits<sqlpp::integer, sqlpp::tag::require_insert>;
    };
    struct PartNumber
    {
      struct _alias_t
      {
        static constexpr const char _literal[] =  "part_number";
        using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
        template<typename T>
        struct _member_t
          {
            T partNumber;
            T& operator()() { return partNumber; }
            const T& operator()() const { return partNumber; }
          };
      };
      using _traits = sqlpp::make_traits<sqlpp::integer, sqlpp::tag::require_insert>;
    };
  } // namespace StorageContents_

  struct StorageContents: sqlpp::table_t<StorageContents,
               StorageContents_::ContentId,
               StorageContents_::Location,
               StorageContents_::Offset,
               StorageContents_::Size,
               StorageContents_::PartNumber>
  {
    struct _alias_t
    {
      static constexpr const char _literal[] =  "storage_contents";
      using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
      template<typename T>
      struct _member_t
      {
        T storageContents;
        T& operator()() { return storageContents; }
        const T& operator()() const { return storageContents; }
      };
    };
  };
} // namespace blimpdb
#endif
