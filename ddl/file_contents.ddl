CREATE TABLE file_contents (
    content_id  INTEGER PRIMARY KEY,
    hash        TEXT    UNIQUE NOT NULL,
);
