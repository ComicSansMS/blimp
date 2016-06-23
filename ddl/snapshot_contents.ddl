CREATE TABLE snapshot_contents (
    snapshot_id INTEGER NOT NULL    REFERENCES snapshot(snapshot_id)        ON UPDATE RESTRICT ON DELETE RESTRICT,
    file_id     INTEGER NOT NULL    REFERENCES file_contents(file_id)       ON UPDATE RESTRICT ON DELETE RESTRICT
);
