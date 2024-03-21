#pragma once

#include <set>
#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>

#include "geo.h"

namespace domain {

struct Stop;

struct Bus {
    Bus() {}
    Bus(const std::string_view& busnm, std::vector<Stop*>& stops, bool is_round)
        : name(std::move(busnm)), route(std::move(stops)), is_roundtrip(is_round)
    {}
    bool IsEmpty() const {
        return name.empty();
    }
    std::string name;
    std::vector<Stop*> route;
    bool is_roundtrip;
};

struct Stop {
    Stop() {}
    Stop(const std::string_view& stopnm, const geo::Coordinates& coords)
        : name(std::move(stopnm)), coordinates(std::move(coords))
    {}
    bool IsEmpty() const {
        return name.empty();
    }
    std::string name;
    geo::Coordinates coordinates;
    std::unordered_set<Bus*> buses;
};

struct BusStat {
    BusStat() {}
    BusStat(const std::string_view& n, int s, int u, double l, double c)
        : name(move(n)), stop_count(s), unique_count(u), length(l), curvature(c) {}

    std::string_view name;
    int stop_count = 0;
    int unique_count = 0;
    double length = 0;
    double curvature = 0; 
};

}