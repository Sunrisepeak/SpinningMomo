// libwebp's encoder, as a module rather than a header.
//
// Encode only: the project writes WebP (thumbnails, gallery assets) and never
// reads it back through libwebp, so <webp/decode.h> is not pulled in here.
//
// Note that `utils::image::WebPEncodeOptions` / `WebPEncodedResult` are the
// PROJECT's own types and have nothing to do with this facade beyond the name.
module;

#include <webp/encode.h>
#include <webp/types.h>

export module sm.vendor.webp;

export {

using ::WebPEncodeBGRA;
using ::WebPEncodeLosslessBGRA;
using ::WebPFree;

}  // export
