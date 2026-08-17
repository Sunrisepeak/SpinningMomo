// spdlog, as a module rather than a header.
//
// The project reaches spdlog only through `utils::logger`, and only through
// ordinary `spdlog::` declarations — no `SPDLOG_*` macro appears anywhere in
// src/ (the one macro that matters, SPDLOG_COMPILED_LIB, is a BUILD contract in
// mcpp.toml / xmake.lua, not something code writes). That is what makes the
// module boundary clean here.
module;

#include <spdlog/sinks/msvc_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/spdlog.h>

export module sm.vendor.spdlog;

export namespace spdlog {

using spdlog::logger;
using spdlog::sink_ptr;
using spdlog::source_loc;
using spdlog::spdlog_ex;
using spdlog::format_string_t;

using spdlog::default_logger;
using spdlog::set_default_logger;
using spdlog::register_logger;
using spdlog::shutdown;

}  // namespace spdlog

export namespace spdlog::level {
using spdlog::level::level_enum;
using spdlog::level::trace;
using spdlog::level::debug;
using spdlog::level::info;
using spdlog::level::warn;
using spdlog::level::err;
using spdlog::level::critical;
using spdlog::level::off;
}  // namespace spdlog::level

export namespace spdlog::sinks {
using spdlog::sinks::msvc_sink_mt;
using spdlog::sinks::rotating_file_sink_mt;
}  // namespace spdlog::sinks
