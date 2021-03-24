CREATE TABLE storage_contents (
    content_id      INTEGER NOT NULL    REFERENCES file_contents(content_id)    ON UPDATE RESTRICT ON DELETE RESTRICT,
    location        TEXT NOT NULL,
    offset          INTEGER,
    size            INTEGER,
    part_number     INTEGER
);
