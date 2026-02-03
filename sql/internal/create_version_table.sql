-- DROP TABLE IF EXISTS versions_;

CREATE TABLE IF NOT EXISTS versions_(
    id INTEGER PRIMARY KEY,
    version INT,
    name TEXT,
    hash TEXT
);
