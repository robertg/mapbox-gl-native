#include <mbgl/tile/tile_id.hpp>

#include <iostream>

namespace mbgl {

::std::ostream& operator<<(::std::ostream& os, const CanonicalTileID& rhs) {
    return os << uint32_t(rhs.z) << "/" << rhs.x << "/" << rhs.y;
}

::std::ostream& operator<<(::std::ostream& os, const OverscaledTileID& rhs) {
    return os << rhs.canonical << "=>" << uint32_t(rhs.overscaledZ);
}

::std::ostream& operator<<(::std::ostream& os, const UnwrappedTileID& rhs) {
    return os << rhs.canonical << (rhs.wrap >= 0 ? "+" : "") << rhs.wrap;
}

} // namespace mbgl
