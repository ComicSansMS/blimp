CREATE TABLE file_contents (
    file_id     INTEGER PRIMARY KEY         REFERENCES file_element(file_id)    ON UPDATE RESTRICT ON DELETE RESTRICT,
    hash        TEXT    UNIQUE NOT NULL,
    hash_type   INTEGER NOT NULL
);
