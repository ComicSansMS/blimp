CREATE TABLE indexed_locations (
    location_id INTEGER PRIMARY KEY,
    path        TEXT    UNIQUE NOT NULL
);
