#ifndef BLIMP_INCLUDE_GUARD_DB_TABLE_FILE_ELEMENT_HPP
#define BLIMP_INCLUDE_GUARD_DB_TABLE_FILE_ELEMENT_HPP

#include <sqlpp11/table.h>
#include <sqlpp11/data_types.h>
#include <sqlpp11/char_sequence.h>

namespace blimpdb
{
  namespace FileElement_
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
            T modifiedTime;
            T& operator()() { return modifiedTime; }
            const T& operator()() const { return modifiedTime; }
          };
      };
      using _traits = sqlpp::make_traits<sqlpp::time_point, sqlpp::tag::require_insert>;
    };
  }

  struct FileElement: sqlpp::table_t<FileElement,
               FileElement_::FileId,
               FileElement_::LocationId,
               FileElement_::ContentId,
               FileElement_::FileSize,
               FileElement_::ModifiedDate>
  {
    struct _alias_t
    {
      static constexpr const char _literal[] =  "file_element";
      using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
      template<typename T>
      struct _member_t
      {
        T fileElement;
        T& operator()() { return fileElement; }
        const T& operator()() const { return fileElement; }
      };
    };
  };
}
#endif
