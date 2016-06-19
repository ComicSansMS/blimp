CREATE TABLE snapshot_contents (
    snapshot_id INTEGER NOT NULL    REFERENCES snapshot(snapshot_id)        ON UPDATE RESTRICT ON DELETE RESTRICT,
    content_id  INTEGER NOT NULL    REFERENCES file_contents(content_id)    ON UPDATE RESTRICT ON DELETE RESTRICT
);
