#pragma once
#include <optional>
#include <iostream>
#include <optional>
#include <memory>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <string>
#include <string_view>
#include <set>

#include "transport_catalogue.h"
#include "transport_router.h"
#include "map_renderer.h"
#include "graph.h"
#include "json.h"
#include "geo.h"
#include "svg.h"
#include "domain.h"

class RequestHandler {
public:
    // MapRenderer понадобится в следующей части итогового проекта
    RequestHandler(
                const t_c::TransportCatalogue& db,
                const renderer::MapRenderer& renderer,
                const TransportRouter& router
    );
    virtual ~RequestHandler() = default;

    // Возвращает информацию о маршруте (запрос Bus)
    std::optional<domain::BusStat> GetBusStat(const std::string_view& bus_name) const;

    // Возвращает маршруты, проходящие через
    std::optional<const std::unordered_set<domain::Bus*>*>
    GetBusesByStop(const std::string_view& stop_name) const;

    void RenderMap(std::ostream& output) const;

    std::optional<graph::Router<double>::RouteInfo> FindRoute(
                            std::string_view stop_from,
                            std::string_view stop_to) const;

    graph::DirectedWeightedGraph<double> GetGraph() const;

    graph::Edge<double> GetEdge(int id) const;

private:
    // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
    const t_c::TransportCatalogue& db_;
    const renderer::MapRenderer& renderer_;
    const TransportRouter& router_;
};
