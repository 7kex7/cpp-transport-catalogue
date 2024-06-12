#include "transport_catalogue.h"

using namespace domain;
 
namespace t_c {

struct TransportCatalogue::Impl {
    Impl() = default;
    Impl(const Impl& other)
        : stops_(other.stops_), stopname_to_stop_(other.stopname_to_stop_)
        , buses_(other.buses_), busname_to_bus_(other.busname_to_bus_)
        , distances_(other.distances_) {
    }

    std::deque<Stop> stops_;
    std::unordered_map<std::string_view, Stop*> stopname_to_stop_;
    std::deque<Bus> buses_;
    std::unordered_map<std::string_view, Bus*> busname_to_bus_;
    std::unordered_map<std::pair<Stop*, Stop*>, double, DistanceHasher> distances_;
};


TransportCatalogue::TransportCatalogue()
    : impl_(std::make_unique<Impl>()) {}

TransportCatalogue& TransportCatalogue::operator=(const TransportCatalogue& other) {
    impl_ = std::unique_ptr<Impl>{ new Impl{*other.impl_}};
    return *this;
}

TransportCatalogue::TransportCatalogue(const TransportCatalogue& other) {
    *this = other;
}

TransportCatalogue& TransportCatalogue::operator=(TransportCatalogue&& other) {
    impl_ = std::move(other.impl_);
    return *this;
}

TransportCatalogue::TransportCatalogue(TransportCatalogue&& other) {
    *this = std::move(other);
}

TransportCatalogue::~TransportCatalogue() = default;

/* ---------------- Distances ---------------- */
void TransportCatalogue::AddDistance(Stop* from_stop, Stop* to_stop, const double distance) {
    impl_->distances_[{from_stop, to_stop}] = std::move(distance);
}

// указанное расстояние double между двумя остановками. Если не найдено: -1
double TransportCatalogue::FindDistance(Stop* first, Stop* second) const {
    auto distance = impl_->distances_.find({first, second});
    if (distance != impl_->distances_.end()) {
        return distance->second;
    }
    auto reverse_distance = impl_->distances_.find({second, first});
    if (reverse_distance != impl_->distances_.end()) {
        return reverse_distance->second;
    }
    return -1;
}

/* ---------------- Stops ---------------- */
void TransportCatalogue::AddStop(const Stop& stop) {
    impl_->stops_.push_back(std::move(stop));
    size_t index = impl_->stops_.size() - 1u;
    Stop* curr_stop_ptr = &impl_->stops_[index];
    impl_->stopname_to_stop_[impl_->stops_[index].name] = curr_stop_ptr;
}

static Stop empty_stop{};

Stop& TransportCatalogue::FindStop(const std::string_view& stopnm) const {
    if (impl_->stopname_to_stop_.find(stopnm) == impl_->stopname_to_stop_.end()) {
        return empty_stop;
    }
    return *impl_->stopname_to_stop_.at(stopnm);
}

const std::unordered_map<std::string_view, Stop*>& 
TransportCatalogue::GetAllStops() const {
    return impl_->stopname_to_stop_;
}

size_t TransportCatalogue::GetStopsCount() const {
    return impl_->stopname_to_stop_.size();
}

/* ---------------- Buses ---------------- */
void TransportCatalogue::AddBus(const Bus& bus) {
    impl_->buses_.push_back(std::move(bus));
    
    size_t index = impl_->buses_.size() - 1u;
    const std::string_view& busnm = impl_->buses_[index].name;
    Bus* const bus_ptr = &impl_->buses_.at(index);
    impl_->busname_to_bus_[busnm] = bus_ptr;

    for (const auto& stop : impl_->buses_[index].route) {
        stop->buses.insert(bus_ptr);
    }
}

static Bus empty_bus{};

Bus& TransportCatalogue::FindBus(const std::string_view& busnm) const {
    if (impl_->busname_to_bus_.find(busnm) == impl_->busname_to_bus_.end()) {
        return empty_bus;
    }
    return *impl_->busname_to_bus_.at(busnm);
}

const std::unordered_map<std::string_view, Bus*>& 
TransportCatalogue::GetAllBuses() const {
    return impl_->busname_to_bus_;
}

} // t_c