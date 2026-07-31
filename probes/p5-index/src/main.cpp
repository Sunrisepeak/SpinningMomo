// P5: the project's own package index, consumed end-to-end.
//
// Five descriptors, chosen to cover every shape the migration needs:
//   sm.xxhash      a C library compiled from a source tarball
//   sm.wil         header-only, buildable via a generated anchor TU
//   sm.sqlite3     a large single-TU C library with feature defines
//   sm.sqlitecpp   a C++ library that DEPENDS on another index package
//   sm.reflectcpp  umbrella TUs + vendored yyjson, the RPC layer's core
//
// Each is exercised through the same facade header the real project uses, so a
// pass here means those `#include`s resolve and those symbols link under MSVC.
#include <xxhash.h>

#include <SQLiteCpp/SQLiteCpp.h>
#include <rfl.hpp>
#include <rfl/json.hpp>
#include <wil/com.h>
#include <wil/resource.h>

#include <cstdio>
#include <string>

struct Probe {
  std::string name;
  int count;
};

int main() {
  // xxhash: content identity, exactly as utils::hash uses it.
  const char* data = "spinningmomo";
  const auto digest = XXH3_64bits(data, 12);

  // SQLiteCpp -> sm.sqlite3 across the dependency edge.
  SQLite::Database db(":memory:", SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
  db.exec("CREATE TABLE t(id INTEGER PRIMARY KEY, name TEXT)");
  db.exec("INSERT INTO t(name) VALUES('probe')");
  SQLite::Statement q(db, "SELECT COUNT(*) FROM t");
  q.executeStep();
  const int rows = q.getColumn(0).getInt();

  // reflect-cpp: the snake_case <-> camelCase round trip core::rpc relies on.
  const Probe p{.name = "p5", .count = rows};
  const std::string json = rfl::json::write(p);
  const auto back = rfl::json::read<Probe>(json);

  // wil: a COM smart pointer, instantiated but never bound.
  wil::com_ptr<IUnknown> unknown;

  std::printf("p5: xxh3=%llu rows=%d json=%s roundtrip=%s wil=%s\n",
              static_cast<unsigned long long>(digest), rows, json.c_str(),
              back && back->name == "p5" ? "ok" : "BAD",
              unknown ? "bound" : "null-ok");

  if (rows != 1 || !back || back->name != "p5" || back->count != 1) {
    std::puts("p5: FAIL");
    return 1;
  }
  std::puts("p5: custom index resolved, all five packages linked");
  return 0;
}
