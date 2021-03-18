#ifndef BLIMP_INCLUDE_GUARD_DB_TABLE_SNAPSHOT_HPP
#define BLIMP_INCLUDE_GUARD_DB_TABLE_SNAPSHOT_HPP

#include <sqlpp11/table.h>
#include <sqlpp11/data_types.h>
#include <sqlpp11/char_sequence.h>

namespace blimpdb
{
  namespace Snapshot_
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
      using _traits = sqlpp::make_traits<sqlpp::integer, sqlpp::tag::must_not_insert, sqlpp::tag::must_not_update>;
    };
    struct Name
    {
      struct _alias_t
      {
        static constexpr const char _literal[] =  "name";
        using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
        template<typename T>
        struct _member_t
          {
            T name;
            T& operator()() { return name; }
            const T& operator()() const { return name; }
          };
      };
      using _traits = sqlpp::make_traits<sqlpp::text, sqlpp::tag::require_insert>;
    };
    struct Date
    {
      struct _alias_t
      {
        static constexpr const char _literal[] =  "date";
        using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
        template<typename T>
        struct _member_t
          {
            T date;
            T& operator()() { return date; }
            const T& operator()() const { return date; }
          };
      };
      using _traits = sqlpp::make_traits<sqlpp::day_point, sqlpp::tag::require_insert>;
    };
  }

  struct Snapshot: sqlpp::table_t<Snapshot,
               Snapshot_::SnapshotId,
               Snapshot_::Name,
               Snapshot_::Date>
  {
    struct _alias_t
    {
      static constexpr const char _literal[] =  "snapshot";
      using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
      template<typename T>
      struct _member_t
      {
        T snapshot;
        T& operator()() { return snapshot; }
        const T& operator()() const { return snapshot; }
      };
    };
  };
}
#endif
