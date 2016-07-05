#ifndef BLIMP_INCLUDE_GUARD_DB_TABLE_SNAPSHOT_CONTENTS_HPP
#define BLIMP_INCLUDE_GUARD_DB_TABLE_SNAPSHOT_CONTENTS_HPP

#include <sqlpp11/table.h>
#include <sqlpp11/data_types.h>
#include <sqlpp11/char_sequence.h>

namespace blimpdb
{
  namespace SnapshotContents_
  {
    struct SnapshotId
    {
      struct _alias_t
      {
        static constexpr const char _literal[] =  "snapshot_id";
        using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
        template<typename T>
        struct _member_t
          {
            T snapshotId;
            T& operator()() { return snapshotId; }
            const T& operator()() const { return snapshotId; }
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
  }

  struct SnapshotContents: sqlpp::table_t<SnapshotContents,
               SnapshotContents_::SnapshotId,
               SnapshotContents_::ContentId>
  {
    struct _alias_t
    {
      static constexpr const char _literal[] =  "snapshot_contents";
      using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
      template<typename T>
      struct _member_t
      {
        T snapshotContents;
        T& operator()() { return snapshotContents; }
        const T& operator()() const { return snapshotContents; }
      };
    };
  };
}
#endif
