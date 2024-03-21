#include "map_renderer.h"

using namespace domain;
using namespace svg;

namespace renderer {
    bool IsZero(double value) {
        return std::abs(value) < EPSILON;
    }

    /* --------------- SphereProjector --------------- */
    SphereProjector::SphereProjector(const SphereProjector& other)
        : padding_(other.padding_), min_lon_(other.min_lon_),
        max_lat_(other.max_lat_), zoom_coeff_(other.zoom_coeff_) {
    }

    svg::Point SphereProjector::operator()(geo::Coordinates coords) const {
        return {
            (coords.lng - min_lon_) * zoom_coeff_ + padding_,
            (max_lat_ - coords.lat) * zoom_coeff_ + padding_
        };
    }

    /* --------------- MapRenderer --------------- */
    MapRenderer::MapRenderer(const RenderSettings& set, const SphereProjector& proj)
        : settings_(set), projector_(proj) {
    }

    svg::Polyline MapRenderer::DrawRoad(Bus* bus_ptr, const svg::Color& color) const {
        svg::Polyline road = svg::Polyline()
                        .SetFillColor(svg::NoneColor)
                        .SetStrokeColor(color)
                        .SetStrokeWidth(settings_.line_width)
                        .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                        .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

        const auto& route = bus_ptr->route;
        for (auto it = route.begin(); it != route.end(); ++it) {
            road.AddPoint(projector_((*it)->coordinates));
        }
        return road;
    }

    std::set<std::string_view> GetBusNames(const std::unordered_map<std::string_view, Bus*>& buses) {
        std::set<std::string_view> names;
        for (const auto& [nm, value] : buses) {
            names.insert(nm);
        }
        return names;
    }

    void MapRenderer::MakeRoadsLayot(BusesMapRef buses
            , const std::set<std::string_view>& sorted_names
            , svg::ObjectContainer& doc) const {

        size_t p_counter = 0;
        auto palette = settings_.color_palette;
        for (const auto& name : sorted_names) {
            Bus* bus = buses.at(name);

            if (bus->route.size() != 0) {
                doc.Add(DrawRoad(bus, palette.at(p_counter)));

                if (p_counter == palette.size() - 1) {
                    p_counter = 0;
                } else {
                    ++p_counter;
                }
            }
        }
    }

    void MapRenderer::SetTextAttrs(svg::Text& text) const {
        text
            .SetFillColor(settings_.underlayer_color)
            .SetStrokeColor(settings_.underlayer_color)
            .SetStrokeWidth(settings_.underlayer_width)
            .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
            .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
    }

    void MapRenderer::SetBaseBusAttrs(Stop* stop_ptr, Bus* bus_ptr, svg::Text& text) const {
        text
            .SetPosition(projector_(stop_ptr->coordinates))
            .SetOffset(settings_.bus_label_offset)
            .SetFontSize(settings_.bus_label_font_size)
            .SetFontFamily("Verdana")
            .SetFontWeight("bold")
            .SetData(bus_ptr->name);
    }
    void MapRenderer::DrawBusName(Bus* bus_ptr, const svg::Color& color
                    , svg::ObjectContainer& doc) const {
        Stop* stop_first = bus_ptr->route.at(0);
        
        svg::Text text_first = svg::Text();
        SetTextAttrs(text_first);
        SetBaseBusAttrs(stop_first, bus_ptr, text_first);
        
        svg::Text overlay_first = svg::Text().SetFillColor(color);
        SetBaseBusAttrs(stop_first, bus_ptr, overlay_first);
        
        doc.Add(text_first);
        doc.Add(overlay_first);

        svg::Text text = svg::Text().SetFillColor("black");
        if (!bus_ptr->is_roundtrip) {
            size_t index = std::floor(bus_ptr->route.size() / 2.0);
            Stop* stop_second = bus_ptr->route.at(index);
            if (stop_first == stop_second) { return; }

            svg::Text text_second = svg::Text();
            SetTextAttrs(text_second);
            SetBaseBusAttrs(stop_second, bus_ptr, text_second);
            
            svg::Text overlay_second = svg::Text().SetFillColor(color);
            SetBaseBusAttrs(stop_second, bus_ptr, overlay_second);
            
            doc.Add(text_second);
            doc.Add(overlay_second);
        }
    }
    void MapRenderer::MakeBusNamesLayot(BusesMapRef buses
            , const std::set<std::string_view>& sorted_names
            , svg::ObjectContainer& doc) const {

        size_t p_counter = 0;
        auto palette = settings_.color_palette;
        for (const auto& name : sorted_names) {
            Bus* bus_ptr = buses.at(name);

            if (bus_ptr->route.size() != 0) {
                DrawBusName(bus_ptr, palette.at(p_counter), doc);

                if (p_counter == palette.size() - 1) {
                    p_counter = 0;
                } else {
                    ++p_counter;
                }
            }
        }
    }

    svg::Circle MapRenderer::DrawCircle(domain::Stop* stop_ptr) const {
        return svg::Circle()
                .SetCenter(projector_(stop_ptr->coordinates))
                .SetRadius(settings_.stop_radius)
                .SetFillColor("white");
    }
    void MapRenderer::MakeCirclesLayot(std::vector<domain::Stop*> stops
            , svg::ObjectContainer& doc) const {
        for (Stop* stop_ptr : stops) {
            doc.Add(DrawCircle(stop_ptr));
        }
    }


    void MapRenderer::SetBaseStopAttrs(domain::Stop* stop_ptr, svg::Text& text) const {
        text
            .SetPosition(projector_(stop_ptr->coordinates))
            .SetOffset(settings_.stop_label_offset)
            .SetFontSize(settings_.stop_label_font_size)
            .SetFontFamily("Verdana")
            .SetData(stop_ptr->name);
    }
    void MapRenderer::DrawStopName(Stop* stop_ptr
                    , svg::ObjectContainer& doc) const {
        svg::Text text = svg::Text();
        SetTextAttrs(text);
        SetBaseStopAttrs(stop_ptr, text);

        svg::Text overlay = svg::Text().SetFillColor("black");
        SetBaseStopAttrs(stop_ptr, overlay);

        doc.Add(text);
        doc.Add(overlay);
    }
    void MapRenderer::MakeStopNamesLayot(std::vector<domain::Stop*> stops
            , svg::ObjectContainer& doc) const {
        for (Stop* stop_ptr : stops) {
            DrawStopName(stop_ptr, doc);
        }
    }

} // namespace renderer