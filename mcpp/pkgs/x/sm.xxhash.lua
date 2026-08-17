-- xxHash — extremely fast non-cryptographic hash. Used by utils::hash for
-- gallery asset content identity.
--
-- Compiled rather than XXH_INLINE_ALL header-only: the project's facade is a
-- plain `#include <xxhash.h>` from several TUs, and the inline mode would
-- re-emit the whole implementation in each of them.
package = {
    spec        = "1",
    namespace   = "sm",
    name        = "xxhash",
    description = "xxHash: extremely fast non-cryptographic hash algorithm",
    licenses    = { "BSD-2-Clause" },
    repo        = "https://github.com/Cyan4973/xxHash",
    type        = "package",

    xpm = {
        windows = {
            ["0.8.3"] = {
                url    = "https://github.com/Cyan4973/xxHash/archive/refs/tags/v0.8.3.tar.gz",
                sha256 = "aae608dfe8213dfd05d909a57718ef82f30722c392344583d3f39050c7f29a80",
            },
        },
        linux = {
            ["0.8.3"] = {
                url    = "https://github.com/Cyan4973/xxHash/archive/refs/tags/v0.8.3.tar.gz",
                sha256 = "aae608dfe8213dfd05d909a57718ef82f30722c392344583d3f39050c7f29a80",
            },
        },
    },

    mcpp = {
        schema       = "0.1",
        language     = "c++23",
        import_std   = false,
        c_standard   = "c11",
        -- Tarball root, so consumers write `#include <xxhash.h>`.
        include_dirs = { "*" },
        sources      = { "*/xxhash.c" },
        targets      = { ["xxhash"] = { kind = "lib" } },
        deps         = { },
    },
}
