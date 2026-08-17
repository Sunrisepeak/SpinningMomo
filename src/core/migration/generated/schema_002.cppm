export module sm.core.migration.generated.schema_002;

import std;

// Auto-generated SQL schema module
// DO NOT EDIT - This file is generated from
// src/migrations/002_watch_root_recovery_state.sql

export namespace core::migration::schema {

struct V002 {
  static constexpr std::array<std::string_view, 1> statements = {
      R"SQL(
CREATE TABLE watch_root_recovery_state (
    root_path TEXT PRIMARY KEY,
    volume_identity TEXT NOT NULL,
    journal_id INTEGER,
    checkpoint_usn INTEGER,
    rule_fingerprint TEXT NOT NULL,
    updated_at INTEGER DEFAULT (unixepoch('subsec') * 1000)
)
        )SQL"};
};

}  // namespace core::migration::schema
