CREATE TABLE file_elements (
    file_id         INTEGER PRIMARY KEY,
    location_id     INTEGER NOT NULL        REFERENCES indexed_locations(location_id)   ON UPDATE RESTRICT ON DELETE RESTRICT,
    content_id      INTEGER NOT NULL        REFERENCES file_contents(content_id)        ON UPDATE RESTRICT ON DELETE RESTRICT,
    file_size       INTEGER NOT NULL,
    modified_date   TEXT    NOT NULL
);
