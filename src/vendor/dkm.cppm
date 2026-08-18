// dkm — a header-only k-means implementation, as a module.
//
// Used by utils::image to reduce a thumbnail to its dominant colours. Both
// exported names are TEMPLATES, so what crosses the module boundary is the
// template itself; the instantiation still happens in the consumer, over the
// consumer's own point type.
module;

#include <dkm.hpp>

export module sm.vendor.dkm;

export namespace dkm {

using dkm::clustering_parameters;
using dkm::kmeans_lloyd;

}  // namespace dkm
