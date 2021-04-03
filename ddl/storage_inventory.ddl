CREATE TABLE storage_inventory (
    content_id      INTEGER NOT NULL    REFERENCES file_contents(content_id)        ON UPDATE RESTRICT ON DELETE RESTRICT,
    container_id    INTEGER NOT NULL    REFERENCES storage_containers(container_id) ON UPDATE RESTRICT ON DELETE RESTRICT,
    offset          INTEGER,
    size            INTEGER,
    part_number     INTEGER,
    PRIMARY KEY (content_id, container_id)
);
