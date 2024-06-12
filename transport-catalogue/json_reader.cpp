#include "json_reader.h"

using namespace domain;
using namespace json;
using namespace renderer;
using namespace t_c;
using namespace std::literals;

namespace json_reader {

FillRequests::FillRequests(t_c::TransportCatalogue& db) 
    : db_(db) {
}

StatRequests::StatRequests(
                            const t_c::TransportCatalogue& db,
                            const renderer::MapRenderer& renderer,
                            const TransportRouter& router
    ) : RequestHandler(db, renderer, router), builder_() {
}

void FillRequests::LoadBusRequest(const Dict& req) {
    const std::string name = req.at("name"s).AsString();
    const bool is_round = req.at("is_roundtrip"s).AsBool();

    const Array& stops_node = req.at("stops"s).AsArray();
    std::vector<std::string> stops_arr;
    stops_arr.reserve(stops_node.size()); 
    for (const Node& nd : stops_node) {
        stops_arr.emplace_back(nd.AsString());
    }
    std::deque<std::string> stops_deq(stops_arr.begin(), stops_arr.end());
    if (!is_round) {
        stops_deq.insert(stops_deq.end()
                , std::next(stops_deq.rbegin()), stops_deq.rend());
    }

    bus_requests_.push_back(BusData{name, stops_deq, is_round});
}
 
void FillRequests::LoadStopRequest(const Dict& req) {

    const std::string name = req.at("name"s).AsString();
    const double latitude = req.at("latitude"s).AsDouble();
    const double longitude = req.at("longitude"s).AsDouble();

    StopData stop{name, latitude, longitude};

    const Dict dists_dict = req.at("road_distances"s).AsDict();
    for (const auto& [stop_name, dist] : dists_dict) {
        stop.road_distances.insert({stop_name, dist.AsDouble()});
    }

    stop_requests_.push_back(stop);
}

void FillRequests::ApplyDistances(Stop& from_stop , RoadDistances& road_distances) {
    for (const auto& [to_name, dist] : road_distances) {
        Stop* const to_stop = &db_.FindStop(to_name);
        db_.AddDistance(&from_stop, to_stop, dist);
    }
}

void FillRequests::ProcessBaseRequests(const Array& arr_reqs) {

    for (const Node& node_map : arr_reqs) {

        const std::string type = node_map.AsDict().at("type"s).AsString();
        if (type == "Bus"s) {
            LoadBusRequest(node_map.AsDict());
        } else if (type == "Stop"s) {
            LoadStopRequest(node_map.AsDict());
        } else {
            throw std::invalid_argument("wrong request type"s);
        }
    }

    for (const auto& stop_data : stop_requests_) {
        db_.AddStop({stop_data.name, stop_data.coords});
    }

    for (const auto& stop_data : stop_requests_) {
        Stop& stop = db_.FindStop(stop_data.name);
        ApplyDistances(stop, stop_data.road_distances);
    }

    for (const auto& bus_data : bus_requests_) {
        std::vector<Stop*> stops_;
        stops_.reserve(bus_data.stops.size());
        for (const auto& stopnm : bus_data.stops) {
            stops_.push_back(&db_.FindStop(stopnm));
        }
        db_.AddBus(Bus{bus_data.name, stops_, bus_data.is_roundtrip});
    }
}

svg::Color GetColorFromNode(const Node& cnode) {
    if (cnode.IsString()) {
        return cnode.AsString();
    } else if (cnode.IsArray()) {
        Array undercolor_arr = cnode.AsArray();
        if (undercolor_arr.size() == 3u) {
            return svg::Rgb(
                            undercolor_arr.at(0).AsInt()
                            , undercolor_arr.at(1).AsInt()
                            , undercolor_arr.at(2).AsInt()
                        );
        } else if (undercolor_arr.size() == 4) {
            return svg::Rgba(
                            undercolor_arr.at(0).AsInt()
                            , undercolor_arr.at(1).AsInt()
                            , undercolor_arr.at(2).AsInt()
                            , undercolor_arr.at(3).AsDouble()
                        );
        } else {
            throw std::invalid_argument("color: invalid array");
        }
    } else {
        throw std::invalid_argument("color: invalid type of node");
    }
}

void FillRequests::ProcessRenderRequests(const Dict& render_settings
                                    , renderer::RenderSettings& settings) {
    settings.width = render_settings.at("width").AsDouble();
    settings.height = render_settings.at("height").AsDouble();
    settings.padding = render_settings.at("padding").AsDouble();
    settings.stop_radius = render_settings.at("stop_radius").AsDouble();
    settings.line_width = render_settings.at("line_width").AsDouble();

    settings.bus_label_font_size = render_settings.at("bus_label_font_size").AsInt();
    Array busl_offset = render_settings.at("bus_label_offset").AsArray();
    settings.bus_label_offset = svg::Point{
                    busl_offset.at(0).AsDouble()
                    , busl_offset.at(1).AsDouble()
                };

    settings.stop_label_font_size = render_settings.at("stop_label_font_size").AsInt();
    Array stopl_offset = render_settings.at("stop_label_offset").AsArray();
    settings.stop_label_offset = svg::Point{
                    stopl_offset.at(0).AsDouble()
                    , stopl_offset.at(1).AsDouble()
                };

    const Node& undercolor_node = render_settings.at("underlayer_color");
    settings.underlayer_color = GetColorFromNode(undercolor_node);
    settings.underlayer_width = render_settings.at("underlayer_width").AsDouble();

    const Array arr_nodes = render_settings.at("color_palette").AsArray();
    for (const Node& node : arr_nodes) {
        svg::Color color = GetColorFromNode(node);
        settings.color_palette.push_back(color);
    }
}

void FillRequests::ProcessRoutingSettings(RoutingSettings& settings, const json::Dict& req) {
    settings.wait_time = req.at("bus_wait_time"s).AsDouble();
    settings.velocity = req.at("bus_velocity"s).AsDouble();
}

// stat requests
void StatRequests::HandleBusRequest(const Dict& req) {

    builder_.StartDict();
    
    const int id = req.at("id").AsInt();
    builder_.Key("request_id"s).Value(id); 

    const std::string busnm = req.at("name").AsString();
    std::optional<BusStat> stat = GetBusStat(busnm);
    if (stat.has_value()) {
        builder_.Key("curvature"s).Value(stat->curvature)
        .Key("route_length"s).Value(stat->length)
        .Key("stop_count"s).Value(stat->stop_count)
        .Key("unique_stop_count"s).Value(stat->unique_count);
    } else {
        builder_.Key("error_message"s).Value("not found"s);
    }
    builder_.EndDict();
}

std::set<std::string> GetSortedBuses(const std::unordered_set<Bus*> routes) {
    std::set<std::string> set_buses;
    for (const auto& bus_ptr : routes) {
        set_buses.insert(bus_ptr->name);
    }

    return set_buses;
}

void StatRequests::HandleStopRequest(const Dict& req) {

    builder_.StartDict();

    const int id = req.at("id").AsInt();
    builder_.Key("request_id"s).Value(id);

    const std::string stopnm = req.at("name").AsString();
    std::optional<const std::unordered_set<Bus*>*> routes = GetBusesByStop(stopnm);
    if (!routes.has_value()) {
        builder_.Key("error_message"s).Value("not found"s);
    } else {
        std::set<std::string> buses = GetSortedBuses(*routes.value());
        builder_.Key("buses"s).StartArray();
        for (const auto& bus : buses) {
            builder_.Value(bus);
        }
        builder_.EndArray();
    }

    builder_.EndDict();
}

void StatRequests::HandleMapRequest(const json::Dict& req) {

    builder_.StartDict();

    const int id = req.at("id"s).AsInt();
    builder_.Key("request_id"s).Value(id);

    std::ostringstream os;
    RenderMap(os);
    builder_.Key("map"s).Value(os.str());

    builder_.EndDict();
}

void StatRequests::HandleRouteRequest(const json::Dict& req) {

    builder_.StartDict();

    const int id = req.at("id"s).AsInt();
    builder_.Key("request_id"s).Value(id);

    std::string stop_from = req.at("from").AsString();
    std::string stop_to = req.at("to").AsString();
    auto route_data = FindRoute(stop_from, stop_to);

    if (!route_data.has_value()) {
        builder_.Key("error_message").Value("not found");
    } else {
        json::Array items;
        auto route = route_data.value();

        items.reserve(route.edges.size());

        double total_time = 0;
        for (const auto& edge_id: route.edges) {
            const graph::Edge<double> edge = GetEdge(edge_id);
            
            json::Builder route_item{};
            route_item.StartDict()
                                 .Key("time"s).Value(edge.weight);
            if (edge.is_wait) {
                route_item
                          .Key("type"s).Value("Wait"s)
                          .Key("stop_name"s).Value(edge.name);
            } else {
                route_item
                          .Key("type"s).Value("Bus"s)
                          .Key("bus"s).Value(edge.name)
                          .Key("span_count"s).Value(static_cast<int>(edge.span));
            }
            route_item.EndDict();
            items.emplace_back(route_item.Build());

            total_time += edge.weight;
        }
        builder_
                .Key("total_time"s).Value(total_time)
                .Key("items"s).Value(items);
    }

    builder_.EndDict();
}

void StatRequests::PrintJsonDocument(const Array& arr_reqs, std::ostream& output) {
    builder_.StartArray();
    for (const Node& req : arr_reqs) {
        const json::Dict& request = req.AsDict();
        const std::string type = request.at("type").AsString();

        if (type == "Bus"s) {
            HandleBusRequest(request);
        } else if (type == "Stop"s) {
            HandleStopRequest(request);
        } else if (type == "Map"s) {
            HandleMapRequest(request);
        } else if (type == "Route"s) {
            HandleRouteRequest(request);
        } else {
            throw std::invalid_argument("wrong request type");
        }
    }
    builder_.EndArray();
    Document out_document(builder_.Build());
    Print(out_document, output);
}

std::vector<geo::Coordinates> GetAllCoordinates(const std::unordered_map<std::string_view, Stop*>& stops) {
    std::vector<geo::Coordinates> coords;
    coords.reserve(stops.size());
    for (const auto& [nm, stop] : stops) {
        if (!stop->buses.empty()) {
            coords.push_back(stop->coordinates);
        }
    }
    return coords;
}

SphereProjector MakeProjector(const TransportCatalogue& db, const RenderSettings& settings) {
    const std::unordered_map<std::string_view, Stop*>& stops = db.GetAllStops();
    std::vector<geo::Coordinates> coords = GetAllCoordinates(stops);
    
    const renderer::SphereProjector proj{
        coords.begin(), coords.end()
        , settings.width
        , settings.height
        , settings.padding
    };

    return proj;
}

void LoadJSON(std::istream& input, std::ostream& output, TransportCatalogue& catalogue) {
    const Document json_document = Load(input);
    assert(json_document.GetRoot().IsDict());

    const Dict all_reqs = json_document.GetRoot().AsDict();

    const Array& base_nd = all_reqs.at("base_requests"s).AsArray();
    FillRequests fill_reqs(catalogue);
    fill_reqs.ProcessBaseRequests(base_nd);

    const Dict& routing_settings_nd = all_reqs.at("routing_settings"s).AsDict();
    RoutingSettings routing_settings;
    fill_reqs.ProcessRoutingSettings(routing_settings, routing_settings_nd);
    TransportRouter router(routing_settings, catalogue);

    const Dict& render_nd = all_reqs.at("render_settings"s).AsDict();
    renderer::RenderSettings settings;
    fill_reqs.ProcessRenderRequests(render_nd, settings);
    SphereProjector projector = MakeProjector(catalogue, settings);
    MapRenderer renderer(settings, projector);

    const Array& stat_nd = all_reqs.at("stat_requests"s).AsArray();
    StatRequests stat_reqs(catalogue, renderer, router);
    stat_reqs.PrintJsonDocument(stat_nd, output);
}

} // json_reader
