CREATE TABLE snapshot_contents (
    snapshot_id INTEGER NOT NULL    REFERENCES snapshots(snapshot_id)       ON UPDATE RESTRICT ON DELETE RESTRICT,
    file_id     INTEGER NOT NULL    REFERENCES file_elements(file_id)       ON UPDATE RESTRICT ON DELETE RESTRICT,
    PRIMARY KEY (snapshot_id, file_id)
);
