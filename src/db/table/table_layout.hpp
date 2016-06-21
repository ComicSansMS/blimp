#ifndef BLIMP_INCLUDE_GUARD_DB_TABLE_TABLE_LAYOUT_HPP
#define BLIMP_INCLUDE_GUARD_DB_TABLE_TABLE_LAYOUT_HPP

namespace blimpdb
{
namespace table_layout
{
inline constexpr char const* blimp_properties()
{
    return R"(
        CREATE TABLE blimp_properties(
            id    TEXT      PRIMARY KEY,
            value TEXT
        );)";
}

inline constexpr char const* file_contents()
{
    return R"(
        CREATE TABLE file_contents (
            content_id  INTEGER PRIMARY KEY,
            file_id     INTEGER NOT NULL            REFERENCES file_element(file_id)
                                                    ON UPDATE RESTRICT ON DELETE RESTRICT,
            hash        TEXT    UNIQUE NOT NULL,
            hash_type   INTEGER NOT NULL
        );)";
}

inline constexpr char const* file_element()
{
    return R"(
        CREATE TABLE file_element (
            file_id         INTEGER PRIMARY KEY,
            location_id     INTEGER NOT NULL        REFERENCES indexed_locations(location_id)
                                                    ON UPDATE RESTRICT ON DELETE RESTRICT,
            size            INTEGER NOT NULL,
            modified_date   TEXT    NOT NULL
        );)";

}

inline constexpr char const* indexed_locations()
{
    return R"(
        CREATE TABLE indexed_locations (
            location_id INTEGER PRIMARY KEY,
            path        TEXT    UNIQUE NOT NULL
        );)";

}

inline constexpr char const* snapshot()
{
    return R"(
        CREATE TABLE snapshot (
            snapshot_id INTEGER PRIMARY KEY,
            name        TEXT    NOT NULL,
            date        TEXT    NOT NULL
        );)";
}

inline constexpr char const* snapshot_contents()
{
    return R"(
        CREATE TABLE snapshot_contents (
            snapshot_id INTEGER NOT NULL    REFERENCES snapshot(snapshot_id)
                                            ON UPDATE RESTRICT ON DELETE RESTRICT,
            content_id  INTEGER NOT NULL    REFERENCES file_contents(content_id)
                                            ON UPDATE RESTRICT ON DELETE RESTRICT
        );)";
}

inline constexpr char const* user_selection()
{
    return R"(
        CREATE TABLE user_selection (
            path    TEXT    UNIQUE NOT NULL
        );)";

}
}
}

#endif
