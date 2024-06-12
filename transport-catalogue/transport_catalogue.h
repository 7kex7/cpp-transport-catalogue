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
    TransportCatalogue(const TransportCatalogue&);
    TransportCatalogue(TransportCatalogue&&);
    ~TransportCatalogue();

    void AddDistance(domain::Stop* fr, domain::Stop* to, const double);
    double FindDistance(domain::Stop* fr, domain::Stop* to) const;

    void AddStop(const domain::Stop&);
    domain::Stop& FindStop(const std::string_view&) const;
    const std::unordered_map<std::string_view, domain::Stop*>& GetAllStops() const;
    size_t GetStopsCount() const;

    void AddBus(const domain::Bus&);
    domain::Bus& FindBus(const std::string_view&) const;
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