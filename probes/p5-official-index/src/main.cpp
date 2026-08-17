// P5: SpinningMomo's whole dependency set, resolved from the PUBLISHED index.
//
// This probe used to consume the project's own path index. Every descriptor in
// it has since been contributed upstream — mcpplibs/mcpp-index#213 through
// #216 — so what it asserts now is the closed loop: a runner that has never
// seen this repository's descriptors can still build against everything the
// project depends on.
//
// Each package is exercised the way the real code exercises it, because
// "resolves" and "links" are different questions from "works":
//
//   chriskohlhoff.asio   as a MODULE, and over the surface #213 added
//   compat.xxhash        content identity, exactly as utils::hash uses it
//   compat.sqlitecpp     …and its dependency edge to compat.sqlite3
//   compat.reflectcpp    the snake_case <-> camelCase round trip core::rpc needs
//   compat.spdlog        compiled mode, i.e. the extern-template path
//   compat.libwebp       a real encode
//   compat.uwebsockets   …and its edges to compat.usockets and compat.libuv
//   compat.wil           a COM smart pointer (Windows-only, gated in the manifest)
//
// Note the two shapes side by side: asio arrives as `import asio;` while
// everything else arrives as headers. That is not an inconsistency — it is the
// project's rule, and the reason is in the root mcpp.toml.
#include <xxhash.h>

#include <SQLiteCpp/SQLiteCpp.h>
#include <rfl.hpp>
#include <rfl/json.hpp>
#include <spdlog/spdlog.h>
#include <webp/encode.h>
#include <wil/com.h>
#include <wil/resource.h>

#if __has_include(<App.h>)
#include <App.h>
#else
#include <uwebsockets/App.h>
#endif

#include <cstdio>
#include <string>
#include <vector>

// Every #include is above every import — the project-wide rule, and it is not
// cosmetic here: <windows.h> arrives through the uWS and WIL headers, and asio's
// BMI carries its own parse of it.
import asio;

struct Probe {
  std::string name;
  int count;
};

int main() {
  // ── compat.xxhash ────────────────────────────────────────────────────────
  const char* data = "spinningmomo";
  const auto digest = XXH3_64bits(data, 12);
  if (digest == 0) return 1;

  // ── compat.sqlitecpp -> compat.sqlite3, across the dependency edge ───────
  SQLite::Database db(":memory:", SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
  db.exec("CREATE TABLE t(id INTEGER PRIMARY KEY, name TEXT)");
  db.exec("INSERT INTO t(name) VALUES('probe')");
  SQLite::Statement q(db, "SELECT COUNT(*) FROM t");
  q.executeStep();
  const int rows = q.getColumn(0).getInt();
  if (rows != 1) return 2;

  // ── compat.reflectcpp ────────────────────────────────────────────────────
  const Probe p{.name = "p5", .count = rows};
  const std::string json = rfl::json::write<rfl::SnakeCaseToCamelCase>(p);
  const auto back = rfl::json::read<Probe, rfl::SnakeCaseToCamelCase>(json);
  if (!back || back->name != "p5" || back->count != 1) return 3;

  // ── compat.spdlog, compiled mode ─────────────────────────────────────────
  // Reaching default_logger() is what pulls the compiled objects in; under
  // header-only mode this would link to a different (inline) definition.
  spdlog::default_logger()->set_level(spdlog::level::warn);
  if (spdlog::default_logger()->level() != spdlog::level::warn) return 4;

  // ── compat.libwebp: a real encode ────────────────────────────────────────
  std::vector<unsigned char> bgra(8 * 8 * 4, 0x40);
  unsigned char* webp = nullptr;
  const std::size_t webp_size = WebPEncodeLosslessBGRA(bgra.data(), 8, 8, 8 * 4, &webp);
  if (webp_size == 0 || webp == nullptr) return 5;
  WebPFree(webp);

  // ── compat.uwebsockets -> compat.usockets -> compat.libuv ────────────────
  // Instantiating App and binding an ephemeral port walks the whole template
  // stack down to a real socket, which is the part a header-only package can
  // still get wrong.
  bool bound = false;
  uWS::App app;
  app.get("/ping", [](auto* res, auto*) { res->end("pong"); });
  app.listen(0, [&bound](us_listen_socket_t* token) {
    bound = token != nullptr;
    if (token) us_listen_socket_close(0, token);
  });
  app.run();
  if (!bound) return 6;

  // ── compat.wil ───────────────────────────────────────────────────────────
  wil::com_ptr<IUnknown> unknown;
  if (unknown) return 7;

  // ── chriskohlhoff.asio, as a module, over the #213 surface ───────────────
  std::error_code ec;
  const auto addr = asio::ip::make_address("127.0.0.1", ec);   // added by #213
  if (ec || !addr.is_loopback()) return 8;
  if (asio::error::make_error_code(asio::error::timed_out)
      != asio::error::timed_out) return 9;                     // added by #213

  asio::io_context io;
  bool posted = false;
  asio::post(io, [&posted] { posted = true; });
  io.run();
  if (!posted) return 10;

  std::printf("p5: the published index carries everything SpinningMomo needs "
              "(xxh3=%llu rows=%d webp=%zu json=%s)\n",
              static_cast<unsigned long long>(digest), rows, webp_size, json.c_str());
  return 0;
}
