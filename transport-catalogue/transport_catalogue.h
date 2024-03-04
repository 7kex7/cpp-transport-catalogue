#pragma once
#include <algorithm>
#include <deque>
#include <set>
#include <string>
#include <string_view>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "geo.h"

namespace t_c {     

struct Stop {
    Stop() {}
    Stop(const std::string_view& stopnm, const Coordinates& coords)
        : name(std::move(stopnm)), coordinates(std::move(coords))
    {}
    bool IsEmpty() const {
        return name.empty();
    }

    std::string name;
    Coordinates coordinates;
    std::set<std::string_view> buses;
};

struct Bus {
    Bus() {}
    Bus(const std::string_view& busnm, std::vector<Stop*>& stops)
        : name(std::move(busnm)), route(std::move(stops))
    {}
    bool IsEmpty() const {
        return name.empty();
    }

    std::string name;
    std::vector<Stop*> route;
};

struct BusInfo {
    BusInfo() {}
    BusInfo(const std::string_view& n, int r, int u, double l, double c)
    : name(move(n)), route_count(r), unique_count(u), length(l), curvature(c) {}

    std::string_view name;
    int route_count = 0;
    int unique_count = 0;
    double length = 0;
    double curvature = 0; 
};


class TransportCatalogue {
public:
    void AddDistance(Stop* from_stop, Stop* to_stop, const double distance);
    double FindDistance(Stop* first, Stop* second) const;
    Stop* AddStop(const Stop& stop);
    Stop& FindStop(const std::string_view& stopnm) const;
    void AddBus(const Bus& bus);
    Bus& FindBus(const std::string_view& bus) const;
    BusInfo GetBusInfo(const std::string_view& bus) const;

    class DistanceHasher {
    static const size_t N = 576UL;
    public:
        size_t operator()(const std::pair<Stop*, Stop*>& element) const {
            size_t hash = std::hash<const void*>{}(element.first);            
            hash += std::hash<const void*>{}(element.second);
            return hash;
        }
    };

private:
    std::deque<Stop> stops_;
    std::unordered_map<std::string_view, Stop*> stopname_to_stop_;
    std::deque<Bus> buses_;
    std::unordered_map<std::string_view, Bus*> busname_to_bus_;
    std::unordered_map<std::pair<Stop*, Stop*>, double, DistanceHasher> distances_;
};

} // t_c