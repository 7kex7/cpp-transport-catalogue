#include "request_handler.h"

using namespace domain;

RequestHandler::RequestHandler(const t_c::TransportCatalogue& db 
                    , const renderer::MapRenderer& renderer)
    : db_(db), renderer_(renderer) {
}

struct StopPtrHasher {
    size_t operator()(const Stop* stop) const {
        return (size_t)stop;
    }
};

std::optional<BusStat> RequestHandler::GetBusStat(const std::string_view& bus_name) const {
    Bus& bus = db_.FindBus(bus_name);
    if (bus.IsEmpty()) {
        return std::nullopt;
    }
    std::vector<Stop*>& route = bus.route;

    // all stops
    int route_count = static_cast<int>(route.size());

    // unique stops
    std::unordered_set<Stop*, StopPtrHasher> unique_stops(bus.route.begin(), bus.route.end());
    int unique_count = static_cast<int>(unique_stops.size());

    // route length
    double geographical_length = 0;
    for (size_t i = 0; i < route.size(); ++i) {
        if (i == route.size() - 1) {
            break;
        }
        geographical_length += geo::ComputeDistance(route[i]->coordinates, route[i + 1]->coordinates);
    }

    double length = 0;
    for (size_t i = 0; i < route.size(); ++i) {
        if (i == route.size() - 1) {
            break;
        }
        length += db_.FindDistance(route[i], route[i + 1]);
    }

    double curvature = length / geographical_length;
    return BusStat{bus_name, route_count, unique_count, length, curvature};
}

std::optional<const std::unordered_set<Bus*>*>
RequestHandler::GetBusesByStop(const std::string_view& stop_name) const {
    const Stop& stop = db_.FindStop(stop_name);
    if (stop.IsEmpty()) {
        return std::nullopt;
    }
    return &stop.buses;
}


std::set<std::string_view> GetBusNames(const std::unordered_map<std::string_view, Bus*>& buses) {
    std::set<std::string_view> names;
    for (const auto& [nm, value] : buses) {
        names.insert(nm);
    }
    return names;
}

// template <typename DrawableIterator>
// void DrawMap(DrawableIterator begin, DrawableIterator end, svg::ObjectContainer& target) {
//     for (auto it = begin; it != end; ++it) {
//         it->Draw(target);
//     }
// }

// template <typename Container>
// void DrawMap(const Container& container, svg::ObjectContainer& target) {
//     using namespace std;
//     DrawMap(begin(container), end(container), target);
// }

std::vector<Stop*> GetStops(const std::unordered_map<std::string_view, Bus*>& buses
                            , const std::unordered_map<std::string_view, Stop*>& stops) {
    std::set<std::string_view> stop_names;
    for (const auto& [name, bus_ptr] : buses) {
        for (const auto& stop_ptr : bus_ptr->route) {
            stop_names.insert(stop_ptr->name);
        }
    }

    std::vector<Stop*> res;
    res.reserve(stop_names.size());
    for (const auto& name : stop_names) {
        res.push_back(stops.at(name));
    }

    return res;
}

void RequestHandler::RenderMap(std::ostream& output) const {
    const std::unordered_map<std::string_view, Bus*>& buses = db_.GetAllBuses();
    auto sorted_names = GetBusNames(buses);
    
    svg::Document doc;
    renderer_.MakeRoadsLayot(buses, sorted_names, doc);
    renderer_.MakeBusNamesLayot(buses, sorted_names, doc);

    const std::unordered_map<std::string_view, Stop*>& stop_ptrs = db_.GetAllStops();
    const auto stops = GetStops(buses, stop_ptrs);
    renderer_.MakeCirclesLayot(stops, doc);
    renderer_.MakeStopNamesLayot(stops, doc);

    doc.Render(output);
}
