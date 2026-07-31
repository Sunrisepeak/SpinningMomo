// P3c: the zero-risk vendor strategy — the facades stay plain headers and each
// consuming module pulls them into its own global module fragment. Entities in
// a GMF attach to the global module, so two modules including the same facade
// share one entity and there is no ODR problem; the only cost is re-parsing
// the SDK header per TU (i.e. exactly what the PCH exists to amortise today).
//
// Nothing here is speculative — if p3a/p3b fail, this is the migration path.
module;

#include "vendor/windows.hpp"

export module feature;

import std;

export namespace feature {

auto screen_width() -> int { return GetSystemMetrics(0 /*SM_CXSCREEN*/); }

auto describe() -> std::string {
  RECT r{0, 0, 4, 8};
  return std::format("gmf-direct rect={}x{}", r.right, r.bottom);
}

}  // namespace feature
