module;

export module core.version;

import std;

export namespace core::version {

inline auto get_app_version() -> std::string { return "2.1.4.0"; }

}  // namespace core::version
