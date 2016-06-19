CREATE TABLE file_element (
    file_id         INTEGER PRIMARY KEY,
    location_id     INTEGER NOT NULL        REFERENCES indexed_locations(location_id)   ON UPDATE RESTRICT ON DELETE RESTRICT,
    size            INTEGER NOT NULL,
    modified_date   TEXT    NOT NULL
);