#pragma once

#include <unordered_map>
#include <string_view>
#include <string>
#include <memory>
#include <vector>

#include "transport_catalogue.h"
#include "domain.h"
#include "graph.h"
#include "router.h"


class TransportRouter {
public:
    TransportRouter(domain::RoutingSettings, const t_c::TransportCatalogue&);

    std::optional<graph::Router<double>::RouteInfo> FindRoute(
                            std::string_view,
                            std::string_view) const;

    const graph::DirectedWeightedGraph<double>& GetGraph() const;

private:
    void Build();

    void InitializeGraphWithStops();

    double ComputeRoadTimeInMinutes(double) const;

    void FillGraph();

private:
    std::unique_ptr< graph::DirectedWeightedGraph<double> > graph_;
    std::unique_ptr< graph::Router<double> > router_;
    std::unordered_map<std::string, size_t> stopnames_to_ids_;

    const domain::RoutingSettings routing_settings_;
    const t_c::TransportCatalogue& catalogue_;
};
