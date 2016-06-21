CREATE TABLE file_contents (
    content_id  INTEGER PRIMARY KEY,
    file_id     INTEGER NOT NULL            REFERENCES file_element(file_id)    ON UPDATE RESTRICT ON DELETE RESTRICT,
    hash        TEXT    UNIQUE NOT NULL,
    hash_type   INTEGER NOT NULL
);
