#ifndef MBGL_TILE_TILE_ID
#define MBGL_TILE_TILE_ID

#include <cstdint>
#include <array>
#include <forward_list>
#include <algorithm>
#include <iosfwd>
#include <cassert>

namespace mbgl {

class OverscaledTileID;
class CanonicalTileID;
class UnwrappedTileID;

// Has integer z/x/y coordinates
// All tiles must be derived from 0/0/0 (=no tiles outside of the main tile pyramid)
// Used for requesting data; represents data tiles that exist out there.
// z is never larger than the source's maxzoom
class CanonicalTileID {
public:
    inline CanonicalTileID(uint8_t z, uint32_t x, uint32_t y);
    inline bool operator==(const CanonicalTileID&) const;
    inline bool operator!=(const CanonicalTileID&) const;
    inline bool operator<(const CanonicalTileID&) const;
    inline bool isChildOf(const CanonicalTileID&) const;
    inline CanonicalTileID scaledTo(uint8_t z) const;
    inline std::array<CanonicalTileID, 4> children() const;

    const uint8_t z;
    const uint32_t x;
    const uint32_t y;
};

::std::ostream& operator<<(::std::ostream& os, const CanonicalTileID& rhs);

// Has integer z/x/y coordinates
// overscaledZ describes the zoom level this tile is intented to represent, e.g. when parsing data
// z is never larger than the source's maxzoom
// z/x/y describe the
class OverscaledTileID {
public:
    inline OverscaledTileID(uint8_t overscaledZ, const CanonicalTileID&);
    inline OverscaledTileID(uint8_t overscaledZ, CanonicalTileID&&);
    inline OverscaledTileID(uint8_t overscaledZ, uint8_t z, uint32_t x, uint32_t y);
    inline OverscaledTileID(uint8_t z, uint32_t x, uint32_t y);
    explicit inline OverscaledTileID(const CanonicalTileID&);
    explicit inline OverscaledTileID(CanonicalTileID&&);
    inline bool operator==(const OverscaledTileID&) const;
    inline bool operator!=(const OverscaledTileID&) const;
    inline bool operator<(const OverscaledTileID&) const;
    inline bool isChildOf(const OverscaledTileID&) const;
    inline uint32_t overscaleFactor() const;
    inline OverscaledTileID scaledTo(uint8_t z) const;
    inline UnwrappedTileID unwrapTo(int16_t wrap) const;

    const uint8_t overscaledZ;
    const CanonicalTileID canonical;
};

::std::ostream& operator<<(::std::ostream& os, const OverscaledTileID& rhs);

// Has integer z/x/y coordinates
// wrap describes tiles that are left/right of the main tile pyramid, e.g. when wrapping the world
// Used for describing what position tiles are getting rendered at (= calc the matrix)
// z is never larger than the source's maxzoom
class UnwrappedTileID {
public:
    inline UnwrappedTileID(uint8_t z, int64_t x, int64_t y);
    inline UnwrappedTileID(int16_t wrap, const CanonicalTileID&);
    inline UnwrappedTileID(int16_t wrap, CanonicalTileID&&);
    inline bool operator==(const UnwrappedTileID&) const;
    inline bool operator!=(const UnwrappedTileID&) const;
    inline bool operator<(const UnwrappedTileID&) const;
    inline bool isChildOf(const UnwrappedTileID&) const;
    inline std::array<UnwrappedTileID, 4> children() const;
    inline OverscaledTileID overscaleTo(uint8_t z) const;

    const int16_t wrap;
    const CanonicalTileID canonical;
};

::std::ostream& operator<<(::std::ostream& os, const UnwrappedTileID& rhs);

CanonicalTileID::CanonicalTileID(uint8_t z_, uint32_t x_, uint32_t y_) : z(z_), x(x_), y(y_) {
    assert(z <= 32);
    assert(x < (1ull << z));
    assert(y < (1ull << z));
}

bool CanonicalTileID::operator==(const CanonicalTileID& rhs) const {
    return z == rhs.z && x == rhs.x && y == rhs.y;
}

bool CanonicalTileID::operator!=(const CanonicalTileID& rhs) const {
    return z != rhs.z || x != rhs.x || y != rhs.y;
}

bool CanonicalTileID::operator<(const CanonicalTileID& rhs) const {
    if (z != rhs.z) {
        return z < rhs.z;
    } else if (x != rhs.x) {
        return x < rhs.x;
    }
    return y < rhs.y;
}

bool CanonicalTileID::isChildOf(const CanonicalTileID& parent) const {
    // We're first testing for z == 0, to avoid a 32 bit shift, which is undefined.
    return parent.z == 0 ||
           (parent.z < z && parent.x == (x >> (z - parent.z)) && parent.y == (y >> (z - parent.z)));
}

CanonicalTileID CanonicalTileID::scaledTo(uint8_t targetZ) const {
    if (targetZ <= z) {
        return { targetZ, x >> (z - targetZ), y >> (z - targetZ) }; // parent or same
    } else {
        return { targetZ, x << (targetZ - z), y << (targetZ - z) }; // child
    }
}

std::array<CanonicalTileID, 4> CanonicalTileID::children() const {
    const uint8_t childZ = z + 1;
    const uint32_t childX = x * 2;
    const uint32_t childY = y * 2;
    return { {
        { childZ, childX, childY },
        { childZ, childX, childY + 1 },
        { childZ, childX + 1, childY },
        { childZ, childX + 1, childY + 1 },
    } };
}

OverscaledTileID::OverscaledTileID(uint8_t overscaledZ_, const CanonicalTileID& canonical_)
    : overscaledZ(overscaledZ_), canonical(canonical_) {
    assert(overscaledZ >= canonical.z);
}

OverscaledTileID::OverscaledTileID(uint8_t overscaledZ_, CanonicalTileID&& canonical_)
    : overscaledZ(overscaledZ_), canonical(std::forward<CanonicalTileID>(canonical_)) {
    assert(overscaledZ >= canonical.z);
}

OverscaledTileID::OverscaledTileID(uint8_t overscaledZ_, uint8_t z, uint32_t x, uint32_t y)
    : overscaledZ(overscaledZ_), canonical(z, x, y) {
    assert(overscaledZ >= canonical.z);
}

OverscaledTileID::OverscaledTileID(uint8_t z, uint32_t x, uint32_t y)
    : overscaledZ(z), canonical(z, x, y) {
}

OverscaledTileID::OverscaledTileID(const CanonicalTileID& canonical_)
    : overscaledZ(canonical_.z), canonical(canonical_) {
    assert(overscaledZ >= canonical.z);
}

OverscaledTileID::OverscaledTileID(CanonicalTileID&& canonical_)
    : overscaledZ(canonical_.z), canonical(std::forward<CanonicalTileID>(canonical_)) {
    assert(overscaledZ >= canonical.z);
}

bool OverscaledTileID::operator==(const OverscaledTileID& rhs) const {
    return overscaledZ == rhs.overscaledZ && canonical == rhs.canonical;
}

bool OverscaledTileID::operator!=(const OverscaledTileID& rhs) const {
    return overscaledZ != rhs.overscaledZ || canonical != rhs.canonical;
}

bool OverscaledTileID::operator<(const OverscaledTileID& rhs) const {
    if (overscaledZ != rhs.overscaledZ) {
        return overscaledZ < rhs.overscaledZ;
    }
    return canonical < rhs.canonical;
}

uint32_t OverscaledTileID::overscaleFactor() const {
    return 1u << (overscaledZ - canonical.z);
}

bool OverscaledTileID::isChildOf(const OverscaledTileID& rhs) const {
    return overscaledZ > rhs.overscaledZ &&
           (canonical == rhs.canonical || canonical.isChildOf(rhs.canonical));
}

OverscaledTileID OverscaledTileID::scaledTo(uint8_t z) const {
    if (z >= canonical.z) {
        return { z, canonical };
    } else {
        return { z, canonical.scaledTo(z) };
    }
}

UnwrappedTileID OverscaledTileID::unwrapTo(int16_t wrap) const {
    return { wrap, canonical };
}

UnwrappedTileID::UnwrappedTileID(uint8_t z_, int64_t x_, int64_t y_)
    : wrap((x_ < 0 ? x_ - (1ll << z_) + 1 : x_) / (1ll << z_)),
      canonical(
          z_,
          static_cast<uint32_t>(x_ - wrap * (1ll << z_)),
          y_ < 0 ? 0 : std::min(static_cast<uint32_t>(y_), static_cast<uint32_t>(1ull << z_) - 1)) {
}

UnwrappedTileID::UnwrappedTileID(int16_t wrap_, const CanonicalTileID& canonical_)
    : wrap(wrap_), canonical(canonical_) {
}

UnwrappedTileID::UnwrappedTileID(int16_t wrap_, CanonicalTileID&& canonical_)
    : wrap(wrap_), canonical(std::forward<CanonicalTileID>(canonical_)) {
}

bool UnwrappedTileID::operator==(const UnwrappedTileID& rhs) const {
    return wrap == rhs.wrap && canonical == rhs.canonical;
}

bool UnwrappedTileID::operator!=(const UnwrappedTileID& rhs) const {
    return wrap != rhs.wrap || canonical != rhs.canonical;
}

bool UnwrappedTileID::operator<(const UnwrappedTileID& rhs) const {
    if (wrap != rhs.wrap) {
        return wrap < rhs.wrap;
    }
    return canonical < rhs.canonical;
}

bool UnwrappedTileID::isChildOf(const UnwrappedTileID& parent) const {
    return wrap == parent.wrap && canonical.isChildOf(parent.canonical);
}

std::array<UnwrappedTileID, 4> UnwrappedTileID::children() const {
    const uint8_t childZ = canonical.z + 1;
    const uint32_t childX = canonical.x * 2;
    const uint32_t childY = canonical.y * 2;
    return { {
        { wrap, { childZ, childX, childY } },
        { wrap, { childZ, childX, childY + 1 } },
        { wrap, { childZ, childX + 1, childY } },
        { wrap, { childZ, childX + 1, childY + 1 } },
    } };
}

OverscaledTileID UnwrappedTileID::overscaleTo(const uint8_t overscaledZ) const {
    assert(overscaledZ >= canonical.z);
    return { overscaledZ, canonical };
}

} // namespace mbgl

#endif
