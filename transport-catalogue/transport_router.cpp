#include "transport_router.h"

using namespace domain;
using namespace t_c;
using namespace graph;

TransportRouter::TransportRouter(RoutingSettings routing_settings,
                                const TransportCatalogue& catalogue)
    : routing_settings_(std::move(routing_settings)), catalogue_(catalogue) {
    Build();
}

std::optional<Router<double>::RouteInfo> TransportRouter::FindRoute(
                        std::string_view stop_from,
                        std::string_view stop_to) const {
    return router_->BuildRoute(
        stopnames_to_ids_.at(std::string(stop_from)),
        stopnames_to_ids_.at(std::string(stop_to))
    );
}

const DirectedWeightedGraph<double>& TransportRouter::GetGraph() const {
    return *graph_;
}

void TransportRouter::Build() {
    // инициализируем граф количеством остановок (вершин) * 2
    graph_ = std::make_unique<DirectedWeightedGraph<double>>(
                                        catalogue_.GetStopsCount()*2
                                    );
    // создаем по 2 вершины на остановку, где вес ребра между - время ожидания
    InitializeGraphWithStops();
    // обозначим время от одной остановки к другой на оном маршруте
    FillGraph();
    router_ = std::make_unique<Router<double>>(*graph_);
}

void TransportRouter::InitializeGraphWithStops() {
    // так как на остановках 2 вершины, 1-я отвечает за ожидание, а вторая - за отправление
    size_t vertex_id = 0;
    for (const auto& [stop_name, stop_ptr] : catalogue_.GetAllStops()) {
        stopnames_to_ids_[std::string(stop_name)] = vertex_id;
        Edge<double> wait_edge{
            vertex_id,
            ++vertex_id,
            routing_settings_.wait_time,
            true,
            0,
            std::string(stop_name)
        };
        graph_->AddEdge(wait_edge);
        ++vertex_id;
    }
}

double TransportRouter::ComputeRoadTimeInMinutes(double distance) const {
    static const double METERS_IN_KILOMETER = 1000;
    static const double MINUTES_IN_HOUR = 60;
    double velocity_meter_minutes = routing_settings_.velocity * METERS_IN_KILOMETER / MINUTES_IN_HOUR;
    return distance / velocity_meter_minutes;
}

void TransportRouter::FillGraph() {
    for (const auto &[bus_name, bus_ptr]: catalogue_.GetAllBuses()) {
        const auto &stops = bus_ptr->route;
        size_t stops_count = stops.size();
        for (size_t i = 0; i < stops_count; ++i) {
            const Stop* stop_from = stops[i];
            size_t dist_sum = 0;
            size_t dist_reverse_sum = 0;

            for (size_t j = i + 1; j < stops_count; ++j) {
                const Stop* stop_to = stops[j];
                dist_sum += catalogue_.FindDistance(stops[j - 1], stops[j]);
                dist_reverse_sum += catalogue_.FindDistance(stops[j], stops[j - 1]);

                const Edge<double> straight_edge {
                    stopnames_to_ids_.at(stop_from->name) + 1,
                    stopnames_to_ids_.at(stop_to->name),
                    ComputeRoadTimeInMinutes(dist_sum),
                    false,
                    static_cast<int>(j - i),
                    bus_ptr->name
                };
                graph_->AddEdge(straight_edge);

                if (!bus_ptr->is_roundtrip) {
                    const Edge<double> reverse_edge {
                        stopnames_to_ids_.at(stop_to->name) + 1,
                        stopnames_to_ids_.at(stop_from->name),
                        ComputeRoadTimeInMinutes(dist_reverse_sum),
                        false,
                        static_cast<int>(j - i),
                        bus_ptr->name
                    };
                    graph_->AddEdge(reverse_edge);
                }
            }
        }
    }
}
