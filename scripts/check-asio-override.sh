#!/usr/bin/env bash
# Is the temporary `sm.asio` override still needed?
#
# It exists only to bridge the window between mcpplibs/mcpp-index#213 landing on
# main and the PUBLISHED index artifact catching up — dependency resolution
# reads the published snapshot, not the index repository's main branch.
#
# This builds a throwaway project against the published `chriskohlhoff.asio` and
# uses the eleven names the amendment added. It runs on Linux, so it costs
# seconds instead of a Windows CI round. File I/O is not asserted: this package
# ships it under ASIO_HAS_FILE, which is Windows-only in this configuration —
# the error/address/connect surface is the part that answers the question, and
# it lands in the same descriptor as the file surface.
#
#   exit 0  -> the published index has it. Delete mcpp/pkgs/a/sm.asio.lua and
#              flip the root mcpp.toml back to [dependencies.chriskohlhoff].
#   exit 1  -> not yet. Keep the override.
#
# FALSE NEGATIVE TO KNOW ABOUT. A descriptor can change WITHOUT its version
# changing — an export-surface amendment is exactly that — and an already
# installed package payload is not invalidated by it. `mcpp index update`
# refreshes the index and the answer here does not move, because resolution
# reuses what is already in the store. On a fresh runner this cannot happen; on
# a developer machine, clear the payload first:
#
#   rm -rf ~/.mcpp/registry/data/xpkgs/chriskohlhoff-x-asio
set -uo pipefail

MCPP=${MCPP:-mcpp}
work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT
mkdir -p "$work/src"

cat > "$work/mcpp.toml" <<'TOML'
[package]
name     = "asiopublishedprobe"
version  = "0.1.0"
standard = "c++23"

[dependencies.chriskohlhoff]
asio = "1.38.1"
TOML

cat > "$work/src/main.cpp" <<'CPP'
import std;
import asio;

int main() {
    // asio::ip::make_address — the parse entry point for the address types the
    // module already exported.
    std::error_code ec;
    const auto addr = asio::ip::make_address("127.0.0.1", ec);
    if (ec || !addr.is_loopback()) return 1;

    // The error enumerators. `ec == asio::error::timed_out` only compiles when
    // the ENUMERATOR's name crossed the module boundary, not just the enum.
    const auto timed = asio::error::make_error_code(asio::error::timed_out);
    if (timed != asio::error::timed_out) return 2;
    if (asio::error::make_error_code(asio::error::connection_refused)
            == asio::error::host_not_found) return 3;

    // The free asio::async_connect over a candidate sequence.
    asio::io_context io;
    asio::ip::tcp::acceptor acceptor(io, {asio::ip::address_v4::loopback(), 0});
    asio::ip::tcp::socket peer(io), sock(io);
    acceptor.async_accept(peer, [](const std::error_code&) {});
    const std::vector<asio::ip::tcp::endpoint> candidates{acceptor.local_endpoint()};
    bool connected = false;
    asio::async_connect(sock, candidates,
        [&](const std::error_code& e, const asio::ip::tcp::endpoint&) { connected = !e; });
    io.run();
    return connected ? 0 : 4;
}
CPP

if (cd "$work" && "$MCPP" run >/dev/null 2>&1); then
    echo "PUBLISHED INDEX IS CURRENT — drop mcpp/pkgs/a/sm.asio.lua and use [dependencies.chriskohlhoff]"
    exit 0
fi
echo "published chriskohlhoff.asio still predates mcpp-index#213 — keep the sm.asio override"
exit 1
