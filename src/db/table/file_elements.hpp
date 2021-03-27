#ifndef BLIMP_INCLUDE_GUARD_DB_TABLE_FILE_ELEMENTS_HPP
#define BLIMP_INCLUDE_GUARD_DB_TABLE_FILE_ELEMENTS_HPP

#include <sqlpp11/table.h>
#include <sqlpp11/data_types.h>
#include <sqlpp11/char_sequence.h>

namespace blimpdb
{
  namespace FileElements_
  {
    struct FileId
    {
      struct _alias_t
      {
        static constexpr const char _literal[] =  "file_id";
        using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
        template<typename T>
        struct _member_t
          {
            T fileId;
            T& operator()() { return fileId; }
            const T& operator()() const { return fileId; }
          };
      };
      using _traits = sqlpp::make_traits<sqlpp::integer, sqlpp::tag::must_not_insert, sqlpp::tag::must_not_update>;
    };
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
      using _traits = sqlpp::make_traits<sqlpp::integer, sqlpp::tag::require_insert>;
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
    struct FileSize
    {
      struct _alias_t
      {
        static constexpr const char _literal[] =  "file_size";
        using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
        template<typename T>
        struct _member_t
          {
            T fileSize;
            T& operator()() { return fileSize; }
            const T& operator()() const { return fileSize; }
          };
      };
      using _traits = sqlpp::make_traits<sqlpp::integer, sqlpp::tag::require_insert>;
    };
    struct ModifiedDate
    {
      struct _alias_t
      {
        static constexpr const char _literal[] =  "modified_date";
        using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
        template<typename T>
        struct _member_t
          {
            T modifiedDate;
            T& operator()() { return modifiedDate; }
            const T& operator()() const { return modifiedDate; }
          };
      };
      using _traits = sqlpp::make_traits<sqlpp::time_point, sqlpp::tag::require_insert>;
    };
  }

  struct FileElements: sqlpp::table_t<FileElements,
               FileElements_::FileId,
               FileElements_::LocationId,
               FileElements_::ContentId,
               FileElements_::FileSize,
               FileElements_::ModifiedDate>
  {
    struct _alias_t
    {
      static constexpr const char _literal[] =  "file_elements";
      using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
      template<typename T>
      struct _member_t
      {
        T fileElements;
        T& operator()() { return fileElements; }
        const T& operator()() const { return fileElements; }
      };
    };
  };
}
#endif
