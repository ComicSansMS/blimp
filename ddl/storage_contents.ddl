CREATE TABLE storage_contents (
    storage_id      INTEGER NOT NULL        REFERENCES storage(storage_id)              ON UPDATE RESTRICT ON DELETE RESTRICT,
    location        TEXT NOT NULL,
    offset          INTEGER,
    size            INTEGER,
    part_no         INTEGER
);
