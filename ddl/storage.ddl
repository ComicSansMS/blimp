CREATE TABLE storage (
    storage_id      INTEGER PRIMARY KEY,
    content_id      INTEGER NOT NULL        REFERENCES file_contents(content_id)        ON UPDATE RESTRICT ON DELETE RESTRICT
);
