// generated by ../external/sqlpp11/scripts/ddl2cpp -fail-on-parse file_element.ddl file_element blimpdb
#ifndef BLIMPDB_FILE_ELEMENT_H
#define BLIMPDB_FILE_ELEMENT_H

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
      using _traits = sqlpp::make_traits<sqlpp::text, sqlpp::tag::require_insert>;
    };
  }

  struct FileElement: sqlpp::table_t<FileElement,
               FileElement_::FileId,
               FileElement_::LocationId,
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
