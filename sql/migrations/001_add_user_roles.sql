ALTER TABLE user_ ADD COLUMN role TEXT DEFAULT 'user';

CREATE INDEX IF NOT EXISTS idx_users_username ON user_(username);
