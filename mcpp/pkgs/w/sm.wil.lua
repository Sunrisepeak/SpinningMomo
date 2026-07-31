-- WIL (Windows Implementation Library) — header-only RAII wrappers over Win32
-- and COM. The project uses wil/com.h (com_ptr), wil/resource.h (unique_*)
-- and wil/result.h.
--
-- Header-only, so a trivial anchor TU gives mcpp a buildable `lib` target —
-- the same shape compat.eigen / compat.opengl use upstream.
package = {
    spec        = "1",
    namespace   = "sm",
    name        = "sm.wil",
    description = "Windows Implementation Library: header-only RAII wrappers for Win32 and COM",
    licenses    = { "MIT" },
    repo        = "https://github.com/microsoft/wil",
    type        = "package",

    xpm = {
        windows = {
            ["1.0.260126.7"] = {
                url    = "https://github.com/microsoft/wil/archive/refs/tags/v1.0.260126.7.tar.gz",
                sha256 = "de9e03b38ff0ff8d22048f00b111cb631d21c550328f12530ccba71c05c9e361",
            },
        },
    },

    mcpp = {
        schema       = "0.1",
        language     = "c++23",
        import_std   = false,
        c_standard   = "c11",
        include_dirs = { "*/include" },
        generated_files = {
            ["mcpp_generated/wil_anchor.c"] = "int mcpp_sm_wil_headers_anchor(void) { return 0; }\n",
        },
        sources      = { "mcpp_generated/wil_anchor.c" },
        targets      = { ["wil"] = { kind = "lib" } },
        deps         = { },
    },
}
