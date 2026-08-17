// xxHash, as a module rather than a header.
//
// The facade's job has not changed — it is still the single place this project
// names an external library — but the mechanism has: the header is parsed ONCE,
// here, and consumers get a BMI instead of a re-parse. Only what the project
// actually calls is exported; adding to this list is how a new dependency on
// xxHash gets noticed in review.
module;

#include <xxhash.h>

export module sm.vendor.xxhash;

export {

using ::XXH64_hash_t;
using ::XXH3_state_t;
using ::XXH_errorcode;
using ::XXH_OK;
using ::XXH_ERROR;

using ::XXH3_64bits;
using ::XXH3_createState;
using ::XXH3_freeState;
using ::XXH3_64bits_reset;
using ::XXH3_64bits_update;
using ::XXH3_64bits_digest;

}  // export
