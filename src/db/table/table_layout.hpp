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

/** The key/value store used by plugins.
 * Keys are composed by concatenating the UUID of the plugin with a key string that is chosen by the plugin.
 */
inline constexpr char const* plugin_kv_store()
{
    return R"(
        CREATE TABLE plugin_kv_store (
            store_key   TEXT      PRIMARY KEY,
            value       BLOB
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
inline constexpr char const* file_elements()
{
    return R"(
        CREATE TABLE file_elements (
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
inline constexpr char const* snapshots()
{
    return R"(
        CREATE TABLE snapshots (
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
            snapshot_id INTEGER NOT NULL    REFERENCES snapshots(snapshot_id)
                                            ON UPDATE RESTRICT ON DELETE RESTRICT,
            file_id     INTEGER NOT NULL    REFERENCES file_elements(file_id)
                                            ON UPDATE RESTRICT ON DELETE RESTRICT,
            PRIMARY KEY (snapshot_id, file_id)
        );)";
}

/** A list of all containers in storage.
 * A container stores a fixed amount of data. Multiple files may share a container and a single file may
 * be split across multiple containers.
 * A storage container may be created without a location, but as soon as it's committed to storage, the location
 * will be filled in. A storage container that is not currently part of an open transaction and has a NULL location
 * nonetheless is considered abandoned and will be garbage-collected eventually.
 */
inline constexpr char const* storage_containers()
{
    return R"(
        CREATE TABLE storage_containers (
            container_id    INTEGER PRIMARY KEY,
            location        TEXT    UNIQUE
        );)";
}

/** The inventory of all individual chunks of data in storage.
 * The contents of a file may be spread across multiple inventory elements with the same content_id.
 * Each element is stored within a container at the specified offset. size is the number of bytes for
 * the respective content at that location, part_number is the 0-based index of the content_element among
 * all the elements with the same content_id.
 */
inline constexpr char const* storage_inventory()
{
    return R"(
        CREATE TABLE storage_inventory (
            content_id      INTEGER NOT NULL    REFERENCES file_contents(content_id)
                                                ON UPDATE RESTRICT ON DELETE RESTRICT,
            container_id    INTEGER NOT NULL    REFERENCES storage_containers(container_id)
                                                ON UPDATE RESTRICT ON DELETE RESTRICT,
            offset          INTEGER,
            size            INTEGER,
            part_number     INTEGER,
            PRIMARY KEY (content_id, container_id)
        );)";
}
}
}
}

#endif
