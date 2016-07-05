#ifndef BLIMP_INCLUDE_GUARD_DB_TABLE_FILE_CONTENTS_HPP
#define BLIMP_INCLUDE_GUARD_DB_TABLE_FILE_CONTENTS_HPP

#include <sqlpp11/table.h>
#include <sqlpp11/data_types.h>
#include <sqlpp11/char_sequence.h>

namespace blimpdb
{
  namespace FileContents_
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
      using _traits = sqlpp::make_traits<sqlpp::integer, sqlpp::tag::must_not_insert, sqlpp::tag::must_not_update>;
    };
    struct Hash
    {
      struct _alias_t
      {
        static constexpr const char _literal[] =  "hash";
        using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
        template<typename T>
        struct _member_t
          {
            T hash;
            T& operator()() { return hash; }
            const T& operator()() const { return hash; }
          };
      };
      using _traits = sqlpp::make_traits<sqlpp::text, sqlpp::tag::require_insert>;
    };
    struct HashType
    {
      struct _alias_t
      {
        static constexpr const char _literal[] =  "hash_type";
        using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
        template<typename T>
        struct _member_t
          {
            T hashType;
            T& operator()() { return hashType; }
            const T& operator()() const { return hashType; }
          };
      };
      using _traits = sqlpp::make_traits<sqlpp::integer, sqlpp::tag::require_insert>;
    };
  }

  struct FileContents: sqlpp::table_t<FileContents,
               FileContents_::ContentId,
               FileContents_::Hash,
               FileContents_::HashType>
  {
    struct _alias_t
    {
      static constexpr const char _literal[] =  "file_contents";
      using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
      template<typename T>
      struct _member_t
      {
        T fileContents;
        T& operator()() { return fileContents; }
        const T& operator()() const { return fileContents; }
      };
    };
  };
}
#endif
