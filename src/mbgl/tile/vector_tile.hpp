#ifndef MBGL_MAP_VECTOR_TILE
#define MBGL_MAP_VECTOR_TILE

#include <mbgl/tile/geometry_tile.hpp>
#include <mbgl/map/tile_id.hpp>
#include <mbgl/util/pbf.hpp>

#include <map>

namespace mbgl {

class VectorTileLayer;

class VectorTileFeature : public GeometryTileFeature {
public:
    VectorTileFeature(pbf, const VectorTileLayer&);

    FeatureType getType() const override { return type; }
    optional<Value> getValue(const std::string&) const override;
    GeometryCollection getGeometries() const override;
    uint32_t getExtent() const override;

private:
    const VectorTileLayer& layer;
    uint64_t id = 0;
    FeatureType type = FeatureType::Unknown;
    pbf tags_pbf;
    pbf geometry_pbf;
};

class VectorTileLayer : public GeometryTileLayer {
public:
    VectorTileLayer(pbf);

    std::size_t featureCount() const override { return features.size(); }
    util::ptr<const GeometryTileFeature> getFeature(std::size_t) const override;

private:
    friend class VectorTile;
    friend class VectorTileFeature;

    std::string name;
    uint32_t extent = 4096;
    std::map<std::string, uint32_t> keys;
    std::vector<Value> values;
    std::vector<pbf> features;
};

class VectorTile : public GeometryTile {
public:
    VectorTile(std::shared_ptr<const std::string> data);

    util::ptr<GeometryTileLayer> getLayer(const std::string&) const override;

private:
    std::shared_ptr<const std::string> data;
    mutable bool parsed = false;
    mutable std::map<std::string, util::ptr<GeometryTileLayer>> layers;
};

class TileID;
class FileSource;

class VectorTileMonitor : public GeometryTileMonitor {
public:
    VectorTileMonitor(const TileID&, float pixelRatio, const std::string& urlTemplate, FileSource&);

    std::unique_ptr<FileRequest> monitorTile(const GeometryTileMonitor::Callback&) override;

private:
    TileID tileID;
    float pixelRatio;
    std::string urlTemplate;
    FileSource& fileSource;
};

} // namespace mbgl

#endif
