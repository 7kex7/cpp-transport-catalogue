#pragma once
#include <algorithm>
#include <deque>
#include <set>
#include <string>
#include <string_view>
#include <stdexcept>
#include <unordered_map>
#include <memory>
#include <unordered_set>
#include <utility>
#include <vector>

#include "geo.h"
#include "domain.h"

namespace t_c {

class TransportCatalogue {
public:
    TransportCatalogue();
    TransportCatalogue(const TransportCatalogue& catalogue);
    TransportCatalogue(TransportCatalogue&& catalogue);
    ~TransportCatalogue();

    void AddDistance(domain::Stop* from_stop, domain::Stop* to_stop, const double distance);
    double FindDistance(domain::Stop* first, domain::Stop* second) const;

    void AddStop(const domain::Stop& stop);
    domain::Stop& FindStop(const std::string_view& stopnm) const;
    const std::unordered_map<std::string_view, domain::Stop*>& GetAllStops() const;

    void AddBus(const domain::Bus& bus);
    domain::Bus& FindBus(const std::string_view& bus) const;
    const std::unordered_map<std::string_view, domain::Bus*>& GetAllBuses() const;

    class DistanceHasher {
    static const size_t N = 576UL;
    public:
        size_t operator()(const std::pair<domain::Stop*, domain::Stop*>& element) const {
            size_t hash = std::hash<const void*>{}(element.first);            
            hash += std::hash<const void*>{}(element.second);
            return hash;
        }
    };

    TransportCatalogue& operator=(const TransportCatalogue& other);
    TransportCatalogue& operator=(TransportCatalogue&& other);
    // передача параметра по значению - копирование

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // t_c