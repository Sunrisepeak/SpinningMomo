// SQLiteCpp, as a module rather than a header.
//
// The wrapper's whole API is ordinary classes — no macros, no ADL-dependent
// free functions — which is what makes it a clean module boundary. The raw
// sqlite3 C API is deliberately NOT re-exported: core::database is written
// against the RAII layer, and anything reaching past it should have to say so.
//
// The `OPEN_*` names are `extern const int` in namespace SQLite (not enumerators
// and not macros), so a using-declaration carries them across intact.
module;

#include <SQLiteCpp/SQLiteCpp.h>

export module sm.vendor.sqlite;

export namespace SQLite {

using SQLite::Column;
using SQLite::Database;
using SQLite::Exception;
using SQLite::Statement;
using SQLite::Transaction;

using SQLite::OPEN_READONLY;
using SQLite::OPEN_READWRITE;
using SQLite::OPEN_CREATE;

}  // namespace SQLite
