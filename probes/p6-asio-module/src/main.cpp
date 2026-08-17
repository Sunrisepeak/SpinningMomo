// Asio consumed as a MODULE, over exactly the surface SpinningMomo calls.
//
// This probe exists because the module form is not a style preference here: with
// asio in each module's global module fragment, MSVC could not reconcile the
// per-importer instantiations of `asio::detail::service_registry::use_service`
// and every consumer of `sm.core.rpc.*` died with
//   fatal error C1116: unrecoverable error importing module '...'
// `import asio;` instantiates them once, inside asio.
//
// The surface asserted below is the intersection of "what the project calls" and
// "what the official wrapper did not export before mcpp-index#213".
import std;
import asio;

int main() {
    // --- ip::make_address (utils/network/network.cpp:201) ---
    std::error_code ec;
    const auto addr = asio::ip::make_address("127.0.0.1", ec);
    if (ec || !addr.is_loopback()) return 1;
    (void)asio::ip::make_address("not-an-address", ec);
    if (!ec) return 2;

    // --- the error conditions network.cpp:17-41 branches on ---
    const auto timed = asio::error::make_error_code(asio::error::timed_out);
    if (timed != asio::error::timed_out) return 3;
    for (const auto& e : {asio::error::make_error_code(asio::error::operation_aborted),
                          asio::error::make_error_code(asio::error::connection_refused),
                          asio::error::make_error_code(asio::error::host_not_found),
                          asio::error::make_error_code(asio::error::host_not_found_try_again),
                          asio::error::make_error_code(asio::error::network_unreachable),
                          asio::error::make_error_code(asio::error::host_unreachable)}) {
        if (!e || e.message().empty()) return 4;
    }

    // --- the free async_connect over a candidate sequence (network.cpp:161) ---
    asio::io_context io;
    asio::ip::tcp::acceptor acceptor(io, {asio::ip::address_v4::loopback(), 0});
    asio::ip::tcp::socket peer(io), sock(io);
    acceptor.async_accept(peer, [](const std::error_code&) {});
    const std::vector<asio::ip::tcp::endpoint> candidates{
        {asio::ip::address_v4::loopback(), 1},   // refused; must be skipped
        acceptor.local_endpoint(),
    };
    bool connected = false;
    asio::async_connect(sock, candidates,
        [&](const std::error_code& e, const asio::ip::tcp::endpoint&) { connected = !e; });
    io.run();
    if (!connected) return 5;

    // --- file I/O: stream_file + random_access_file + file_base flags ---
    // (utils/file/file.cpp:109,209 and core/http_server/{types,static})
    //
    // Guarded by _WIN32, not ASIO_HAS_FILE: a macro does not cross a module
    // boundary, and a module consumer includes no asio header, so ASIO_HAS_FILE
    // is always false in THIS TU regardless of how the package was built. For
    // this package's configuration the two coincide anyway — io_uring is off, so
    // the only file backend is Windows IOCP. Keeping the guard here is what lets
    // the probe also run under Linux/gcc locally, which is where every other
    // assertion below gets checked before a CI round is spent.
#if defined(_WIN32)
    const auto path = (std::filesystem::temp_directory_path() / "p6_asio_probe.bin").string();
    std::error_code rm;
    std::filesystem::remove(path, rm);
    constexpr std::string_view payload = "spinningmomo";

    asio::io_context fio;
    int file_failure = 0;
    asio::co_spawn(fio, [&]() -> asio::awaitable<void> {
        {
            asio::stream_file out(co_await asio::this_coro::executor, path,
                                  asio::file_base::write_only | asio::file_base::create
                                      | asio::file_base::truncate);
            if (co_await asio::async_write(out, asio::buffer(payload), asio::use_awaitable)
                != payload.size()) { file_failure = 6; co_return; }
        }
        {
            asio::random_access_file ra(co_await asio::this_coro::executor, path,
                                        asio::file_base::read_only);
            std::string tail(5, '\0');                       // "momo" is 4; read "gmomo"
            if (co_await ra.async_read_some_at(7, asio::buffer(tail), asio::use_awaitable) != 5
                || tail != "gmomo") { file_failure = 7; co_return; }
        }
    }, asio::detached);
    fio.run();
    std::filesystem::remove(path, rm);
    if (file_failure) return file_failure;
#endif  // _WIN32

    // --- the awaitable alias shape core::rpc builds its handlers on ---
    static_assert(std::is_same_v<asio::awaitable<int>, asio::awaitable<int>>);
    static_assert(std::is_class_v<asio::any_io_executor>);
    static_assert(std::is_same_v<asio::error_code, std::error_code>);

    std::println("p6: asio module surface OK");
    return 0;
}
