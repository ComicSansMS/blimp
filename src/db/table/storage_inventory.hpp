#ifndef BLIMP_INCLUDE_GUARD_DB_TABLE_STORAGE_INVENTORY_HPP
#define BLIMP_INCLUDE_GUARD_DB_TABLE_STORAGE_INVENTORY_HPP

#include <sqlpp11/table.h>
#include <sqlpp11/data_types.h>
#include <sqlpp11/char_sequence.h>

namespace blimpdb
{
  namespace StorageInventory_
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
      using _traits = sqlpp::make_traits<sqlpp::integer, sqlpp::tag::require_insert>;
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
      using _traits = sqlpp::make_traits<sqlpp::integer, sqlpp::tag::can_be_null>;
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
      using _traits = sqlpp::make_traits<sqlpp::integer, sqlpp::tag::can_be_null>;
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
      using _traits = sqlpp::make_traits<sqlpp::integer, sqlpp::tag::can_be_null>;
    };
  } // namespace StorageInventory_

  struct StorageInventory: sqlpp::table_t<StorageInventory,
               StorageInventory_::ContentId,
               StorageInventory_::ContainerId,
               StorageInventory_::Offset,
               StorageInventory_::Size,
               StorageInventory_::PartNumber>
  {
    struct _alias_t
    {
      static constexpr const char _literal[] =  "storage_inventory";
      using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
      template<typename T>
      struct _member_t
      {
        T storageInventory;
        T& operator()() { return storageInventory; }
        const T& operator()() const { return storageInventory; }
      };
    };
  };
} // namespace blimpdb
#endif
