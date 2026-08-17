-- TEMPORARY OVERRIDE of the official `chriskohlhoff.asio`.
--
-- WHY IT EXISTS. The project consumes Asio as a module (`import asio;`), and the
-- official descriptor's export surface was missing eleven names this codebase
-- actually calls:
--
--   asio::stream_file / random_access_file / file_base   utils/file/file.cpp,
--                                                        core/http_server/{types,static}
--   asio::error::{timed_out, host_not_found,             utils/network/network.cpp:17-41
--     host_not_found_try_again, host_unreachable,
--     network_unreachable, connection_refused}
--   asio::ip::make_address                               utils/network/network.cpp:201
--   asio::async_connect (the free function)              utils/network/network.cpp:161
--
-- The amendment is UPSTREAM AND MERGED: mcpplibs/mcpp-index#213. This file only
-- bridges the window between that merge and the published index artifact
-- catching up — resolution reads the published snapshot, not the index's main.
--
-- A package name and a MODULE name are independent: this ships as `sm.asio` and
-- still provides the module `asio`, so project code never mentions either
-- spelling — it just writes `import asio;`.
--
-- WHEN TO DELETE IT. As soon as `chriskohlhoff.asio@1.38.1` resolves from the
-- published index WITH these names. Flip the root mcpp.toml back to
-- `[dependencies.chriskohlhoff]` and remove this file;
-- scripts/check-asio-override.sh answers "is it time yet" without a CI round.
--
-- The `mcpp = {}` table below is a VERBATIM copy of the merged upstream
-- descriptor. Do not edit it here — edit it upstream and re-copy, or the two
-- silently diverge and the project stops testing what it will ship against.
--
package = {
    spec        = "1",
    namespace   = "sm",
    name        = "asio",
    description = "Standalone asio exposed as the C++23 module `asio` (separate compilation)",
    licenses    = {"BSL-1.0"},
    repo        = "https://github.com/chriskohlhoff/asio",
    type        = "package",

    xpm = {
        linux = {
            ["1.38.1"] = {
                url = {
                    GLOBAL = "https://github.com/chriskohlhoff/asio/archive/refs/tags/asio-1-38-1.tar.gz",
                    CN     = "https://gitcode.com/mcpp-res/asio/releases/download/1.38.1/asio-1.38.1.tar.gz",
                },
                sha256 = "2827b229972be80cdb14e5497962fa393d1adf036b5869e2b9c99f644daadacc",
            },
        },
        macosx = {
            ["1.38.1"] = {
                url = {
                    GLOBAL = "https://github.com/chriskohlhoff/asio/archive/refs/tags/asio-1-38-1.tar.gz",
                    CN     = "https://gitcode.com/mcpp-res/asio/releases/download/1.38.1/asio-1.38.1.tar.gz",
                },
                sha256 = "2827b229972be80cdb14e5497962fa393d1adf036b5869e2b9c99f644daadacc",
            },
        },
        windows = {
            -- Upstream tag archives carry two POSIX symlinks
            -- (asio/include -> ../include, asio/src -> ../src) that tar.exe
            -- cannot materialize on the Windows runner. This uses the existing
            -- symlink-free repack documented by xlings-res/asio.
            ["1.38.1"] = {
                url = {
                    GLOBAL = "https://github.com/xlings-res/asio/releases/download/1.38.1/asio-1.38.1-nosymlinks.tar.gz",
                    CN     = "https://gitcode.com/mcpp-res/asio/releases/download/1.38.1/asio-1.38.1-nosymlinks.tar.gz",
                },
                sha256 = "77f74094bb12cd867a6edbf5736bbed816c6ce0906e880de8573097a81714d89",
            },
        },
    },

    mcpp = {
        schema       = "0.1",
        language     = "c++23",
        import_std   = false,
        modules      = { "asio" },
        -- GitHub wraps the tag as asio-asio-1-38-1/; expose its include root
        -- so the wrapper's `#include <asio/*.hpp>` resolve.
        include_dirs = { "*/include" },
        generated_files = {
            ["mcpp_generated/asio.cppm"] = [==[
module;
#include <asio/io_context.hpp>
#include <asio/post.hpp>
#include <asio/executor_work_guard.hpp>
#include <asio/dispatch.hpp>
#include <asio/defer.hpp>
#include <asio/steady_timer.hpp>
#include <asio/thread_pool.hpp>
#include <asio/strand.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/ip/address_v4.hpp>
#include <asio/buffer.hpp>
#include <asio/awaitable.hpp>
#include <asio/this_coro.hpp>
#include <asio/use_awaitable.hpp>
#include <asio/co_spawn.hpp>
#include <asio/cancellation_signal.hpp>
#include <asio/cancellation_type.hpp>
#include <asio/bind_cancellation_slot.hpp>
#include <asio/execution_context.hpp>
#include <asio/any_io_executor.hpp>
#include <asio/system_executor.hpp>
#include <asio/system_context.hpp>
#include <asio/associated_executor.hpp>
#include <asio/associated_allocator.hpp>
#include <asio/associated_cancellation_slot.hpp>
#include <asio/error_code.hpp>
#include <asio/detached.hpp>
#include <asio/use_future.hpp>
#include <asio/deferred.hpp>
#include <asio/redirect_error.hpp>
#include <asio/bind_executor.hpp>
#include <asio/signal_set.hpp>
#include <asio/system_timer.hpp>
#include <asio/bind_allocator.hpp>
#include <asio/append.hpp>
#include <asio/prepend.hpp>
#include <asio/consign.hpp>
#include <asio/as_tuple.hpp>
#include <asio/socket_base.hpp>
#include <asio/connect.hpp>
#include <asio/read.hpp>
#include <asio/write.hpp>
#include <asio/read_until.hpp>
#include <asio/ip/udp.hpp>
#include <asio/ip/address.hpp>
#include <asio/ip/address_v6.hpp>
#include <asio/error.hpp>
// File I/O exists only where Asio has a backend for it: IOCP on Windows, or
// io_uring on Linux (asio/detail/config.hpp, "// Files."). ASIO_HAS_FILE is
// Asio's own answer to that question, so the guard here is the library's, not
// a platform list this descriptor would have to keep in sync.
#if defined(ASIO_HAS_FILE)
#include <asio/file_base.hpp>
#include <asio/stream_file.hpp>
#include <asio/random_access_file.hpp>
#endif
#include <asio/experimental/promise.hpp>
#include <asio/experimental/channel_error.hpp>
#include <asio/experimental/channel.hpp>
#include <asio/experimental/concurrent_channel.hpp>
#include <asio/experimental/use_promise.hpp>
#include <asio/experimental/parallel_group.hpp>
#include <asio/experimental/awaitable_operators.hpp>
#ifdef MCPP_FEATURE_SSL
#include <asio/ssl.hpp>
#include <asio/ssl/context.hpp>
#include <asio/ssl/stream.hpp>
#include <asio/ssl/error.hpp>
#endif

export module asio;

export namespace asio::detail {
using ::std::chrono::operator==;
using ::std::chrono::operator<;
using ::std::chrono::operator>=;
using ::std::chrono::operator+;
using ::std::chrono::operator-;
using ::std::coroutine_traits;
}

export namespace asio::error {
using ::asio::error::make_error_code;
// The four error enums and their enumerators. Exporting `operation_aborted`
// alone made every OTHER condition unreachable through the module — a consumer
// that wants to tell a connection refusal from a DNS failure had no name to
// compare against. `using enum` keeps that from becoming a hand-maintained
// list of ~40 enumerators that drifts on the next Asio release.
using ::asio::error::basic_errors;
using ::asio::error::netdb_errors;
using ::asio::error::addrinfo_errors;
using ::asio::error::misc_errors;
using enum ::asio::error::basic_errors;
using enum ::asio::error::netdb_errors;
using enum ::asio::error::addrinfo_errors;
using enum ::asio::error::misc_errors;
}

export namespace asio {
using ::asio::io_context;
using ::asio::post;
using ::asio::make_work_guard;
using ::asio::dispatch;
using ::asio::defer;
using ::asio::steady_timer;
using ::asio::thread_pool;
using ::asio::make_strand;
using ::asio::mutable_buffer;
using ::asio::const_buffer;
using ::asio::buffer;
using ::asio::awaitable;
using ::asio::use_awaitable;
using ::asio::co_spawn;
using ::asio::cancellation_signal;
using ::asio::cancellation_type;
using ::asio::bind_cancellation_slot;
using ::asio::execution_context;
using ::asio::any_io_executor;
using ::asio::system_executor;
using ::asio::system_context;
using ::asio::associated_executor;
using ::asio::associated_allocator;
using ::asio::associated_cancellation_slot;
using ::asio::error_code;
using ::asio::detached;
using ::asio::detached_t;
using ::asio::use_future;
using ::asio::deferred;
using ::asio::deferred_t;
using ::asio::redirect_error;
using ::asio::bind_executor;
using ::asio::signal_set;
using ::asio::system_timer;
using ::asio::bind_allocator;
using ::asio::append;
using ::asio::prepend;
using ::asio::consign;
using ::asio::as_tuple;
using ::asio::socket_base;
using ::asio::connect;
using ::asio::async_connect;
using ::asio::async_read;
using ::asio::async_write;
using ::asio::read;
using ::asio::write;
using ::asio::read_until;
}

#if defined(ASIO_HAS_FILE)
export namespace asio {
using ::asio::file_base;
using ::asio::basic_file;
using ::asio::basic_stream_file;
using ::asio::basic_random_access_file;
using ::asio::stream_file;
using ::asio::random_access_file;
}
#endif

export namespace asio::experimental {
using ::asio::experimental::channel;
using ::asio::experimental::concurrent_channel;
using ::asio::experimental::use_promise;
}

export namespace asio::experimental::error {
using ::asio::experimental::error::make_error_code;
}

export namespace asio::ip {
using ::asio::ip::tcp;
using ::asio::ip::udp;
using ::asio::ip::address;
using ::asio::ip::address_v4;
using ::asio::ip::address_v6;
using ::asio::ip::make_address;
using ::asio::ip::make_address_v4;
using ::asio::ip::make_address_v6;
}

export namespace asio::this_coro {
using ::asio::this_coro::executor;
using ::asio::this_coro::cancellation_state;
using ::asio::this_coro::throw_if_cancelled;
using ::asio::this_coro::reset_cancellation_state;
}

#ifdef MCPP_FEATURE_SSL
export namespace asio::ssl {
using ::asio::ssl::context;
using ::asio::ssl::context_base;
using ::asio::ssl::stream;
using ::asio::ssl::stream_base;
using ::asio::ssl::verify_context;
using ::asio::ssl::verify_mode;
using ::asio::ssl::host_name_verification;
}

export namespace asio::ssl::error {
using ::asio::ssl::error::stream_errors;
using ::asio::ssl::error::make_error_code;
// stream_category is static const ref (internal linkage) — can't export.
}
#endif

]==],
        },
        sources = {
            "mcpp_generated/asio.cppm",
            "*/src/asio.cpp",
        },
        targets = { ["asio"] = { kind = "lib" } },
        -- `separate-compilation` is a default feature so its defines propagate
        -- to every consumer TU (the module BMI and the consumer must agree on
        -- ASIO_SEPARATE_COMPILATION or the inline/extern split miscompiles).
        --
        -- ASIO_HAS_THREADS: asio's detection keys off CRT macros
        -- (_MT/_REENTRANT/_POSIX_THREADS) that the workspace's llvm-on-Windows
        -- toolchain does not define, otherwise silently selecting null_thread.
        -- Pin the known multithreaded package contract; POSIX pthread selection
        -- still runs beneath this define where applicable.
        features = {
            ["default"] = { implies = { "separate-compilation" } },
            ["separate-compilation"] = {
                defines = {
                    "ASIO_STANDALONE",
                    "ASIO_SEPARATE_COMPILATION",
                    "ASIO_DISABLE_BOOST_CONTEXT_FIBER",
                    "ASIO_HAS_THREADS",
                    "ASIO_NO_IOSTREAM",
                },
            },
            ["ssl"] = {
                defines = { "MCPP_FEATURE_SSL" },
                deps    = { ["compat.openssl"] = "3.5.1" },
                sources = { "*/src/asio_ssl.cpp" },
            },
        },
        deps = {},
        -- POSIX threading is detected by asio from unistd.h feature macros;
        -- retain the portable driver-level thread link contract on Linux.
        linux = {
            ldflags = { "-pthread" },
        },
        -- On the supported desktop MSVC-ABI route, asio autolinks ws2_32.lib
        -- and mswsock.lib. Do not inject GNU -l flags into native link.exe.
    },
}
