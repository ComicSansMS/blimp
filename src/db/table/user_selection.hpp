#ifndef BLIMP_INCLUDE_GUARD_DB_TABLE_USER_SELECTION_HPP
#define BLIMP_INCLUDE_GUARD_DB_TABLE_USER_SELECTION_HPP

#include <sqlpp11/table.h>
#include <sqlpp11/data_types.h>
#include <sqlpp11/char_sequence.h>

namespace blimpdb
{
  namespace UserSelection_
  {
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

  struct UserSelection: sqlpp::table_t<UserSelection,
               UserSelection_::Path>
  {
    struct _alias_t
    {
      static constexpr const char _literal[] =  "user_selection";
      using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
      template<typename T>
      struct _member_t
      {
        T userSelection;
        T& operator()() { return userSelection; }
        const T& operator()() const { return userSelection; }
      };
    };
  };
}
#endif
