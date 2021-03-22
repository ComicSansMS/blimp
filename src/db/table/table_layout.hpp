#ifndef BLIMP_INCLUDE_GUARD_DB_TABLE_TABLE_LAYOUT_HPP
#define BLIMP_INCLUDE_GUARD_DB_TABLE_TABLE_LAYOUT_HPP

namespace blimpdb
{
namespace table_layout
{
inline namespace v10000
{
/** A key/value store for saving generic properties.
 */
inline constexpr char const* blimp_properties()
{
    return R"(
        CREATE TABLE blimp_properties(
            id    TEXT      PRIMARY KEY,
            value TEXT
        );)";
}

/** A list of paths selected by the user on the UI.
 * The user_selection is only the minimal information that is required for serializing the UI state.
 * It stores for each branch in the tree only the highest item that is 'Checked'.
 */
inline constexpr char const* user_selection()
{
    return R"(
        CREATE TABLE user_selection (
            path    TEXT    UNIQUE NOT NULL
        );)";
}

/** A list of physical locations on disk.
 * A list of all the files in the file index.
 * Each indexed file may map to one or more file_element.
 */
inline constexpr char const* indexed_locations()
{
    return R"(
        CREATE TABLE indexed_locations (
            location_id INTEGER PRIMARY KEY,
            path        TEXT    UNIQUE NOT NULL
        );)";
}

/** A list of physical file contents.
 * Represents the actual content (ie. the bytes stored) of a file.
 * More than one file_element may share the same content (eg. all empty files have the same content).
 */
inline constexpr char const* file_contents()
{
    return R"(
        CREATE TABLE file_contents (
            content_id  INTEGER PRIMARY KEY,
            hash        TEXT    UNIQUE NOT NULL
        );)";
}

/** A list of physical file states.
* Together with the corresponding file_contents, a file_element represents an actual physical file that was scanned
* from an indexed_location at one point.
* file_element stores only the metadata relevant for indexing. The actual content of the file is represented
* by file_contents.
*/
inline constexpr char const* file_element()
{
    return R"(
        CREATE TABLE file_element (
            file_id         INTEGER PRIMARY KEY,
            location_id     INTEGER NOT NULL        REFERENCES indexed_locations(location_id)
                                                    ON UPDATE RESTRICT ON DELETE RESTRICT,
            content_id      INTEGER NOT NULL        REFERENCES file_contents(content_id)
                                                    ON UPDATE RESTRICT ON DELETE RESTRICT,
            file_size       INTEGER NOT NULL,
            modified_date   TEXT    NOT NULL
        );)";
}

/** A list of snapshots. A snapshot is a set of file_contents.
 */
inline constexpr char const* snapshot()
{
    return R"(
        CREATE TABLE snapshot (
            snapshot_id INTEGER PRIMARY KEY,
            name        TEXT    NOT NULL,
            date        TEXT    NOT NULL
        );)";
}

/** The contents of all snapshots.
 */
inline constexpr char const* snapshot_contents()
{
    return R"(
        CREATE TABLE snapshot_contents (
            snapshot_id INTEGER NOT NULL    REFERENCES snapshot(snapshot_id)
                                            ON UPDATE RESTRICT ON DELETE RESTRICT,
            file_id     INTEGER NOT NULL    REFERENCES file_element(file_id)
                                            ON UPDATE RESTRICT ON DELETE RESTRICT
        );)";
}

/** Maps a file content to a storage id. To retrieve a file_element, first retrieve the corresponding storage_id
 *  for its contents from this table, then fetch all of the items with a matching storage_id from storage_contents.
 */
inline constexpr char const* storage()
{
    return R"(
        CREATE TABLE storage (
            storage_id      INTEGER PRIMARY KEY,
            content_id      INTEGER NOT NULL        REFERENCES file_contents(content_id)
                                                    ON UPDATE RESTRICT ON DELETE RESTRICT
        );)";
}

/** A single chunk of data in storage.
 * The contents of a file may be spread across multiple storage_contents elements with the same storage_id.
 * Each element is located in storage by a location string and an offset for that location. size is the number of
 * bytes for the respective content at that location, part_no is the 0-based index of the content_element among all
 * the elements with the same storage_id.
 */
inline constexpr char const* storage_contents()
{
    return R"(
        CREATE TABLE storage_contents (
            storage_id      INTEGER NOT NULL        REFERENCES storage(storage_id)
                                                    ON UPDATE RESTRICT ON DELETE RESTRICT,
            location        TEXT NOT NULL,
            offset          INTEGER,
            size            INTEGER,
            part_no         INTEGER
        );)";
}

}
}
}

#endif
