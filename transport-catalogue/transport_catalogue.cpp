#include "transport_catalogue.h"

namespace t_c {


void TransportCatalogue::AddStop(const Stop& stop) {
    stops_.push_back(std::move(stop));
    size_t index = stops_.size() - 1u;
    Stop* curr_stop_ptr = &stops_[index];
    stopname_to_stop_[stops_[index].name] = curr_stop_ptr;

    // for (const auto& [name, stop_ptr] : stopname_to_stop_) {
    //     double distance = ComputeDistance(curr_stop_ptr->coordinates, stop_ptr->coordinates);
    //     distances_[std::make_pair(curr_stop_ptr, stop_ptr)] = distance;
    // }
}

static Stop empty_stop{};

Stop& TransportCatalogue::FindStop(const std::string_view& stopnm) const {
    if (stopname_to_stop_.find(stopnm) == stopname_to_stop_.end()) {
        return empty_stop;
    }
    return *stopname_to_stop_.at(stopnm);
}


void TransportCatalogue::AddBus(const Bus& bus) {
    buses_.push_back(std::move(bus));
    size_t index = buses_.size() - 1u;
    const std::string_view& busnm = buses_[index].name;
    busname_to_bus_[busnm] = &buses_.at(index);

    for (const auto& stop : buses_[index].route) {
        stop->buses.insert(busnm);
    }
}


static Bus empty_bus{};

Bus& TransportCatalogue::FindBus(const std::string_view& busnm) const {
    if (busname_to_bus_.find(busnm) == busname_to_bus_.end()) {
        return empty_bus;
    }
    return *busname_to_bus_.at(busnm);
}

BusInfo TransportCatalogue::GetBusInfo(const std::string_view& busnm) const {
    Bus& bus = FindBus(busnm);
    if (bus.IsEmpty()) {
        return BusInfo{};
    }
    std::vector<Stop*>& route = bus.route;

    // all stops
    int R = route.size();

    // unique stops
    std::unordered_set<Stop*, StopPtrHasher> unique_stops(bus.route.begin(), bus.route.end());
    int U = unique_stops.size();

    // route length
    double L = 0;
    for (size_t i = 0; i < route.size(); ++i) {
        if (i == route.size() - 1) {
            break;
        }
        L += ComputeDistance(route[i]->coordinates, route[i + 1]->coordinates);
    }
    return BusInfo{busnm, R, U, L};
}

} // t_c