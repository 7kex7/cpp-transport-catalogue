#pragma once
#include <optional>
#include <iostream>
#include <memory>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <string>
#include <string_view>
#include <set>

#include "transport_catalogue.h"
#include "map_renderer.h"
#include "json.h"
#include "geo.h"
#include "svg.h"
#include "domain.h"

class RequestHandler {
public:
    // MapRenderer понадобится в следующей части итогового проекта
    RequestHandler(const t_c::TransportCatalogue& db, const renderer::MapRenderer& renderer);
    virtual ~RequestHandler() = default;

    // Возвращает информацию о маршруте (запрос Bus)
    std::optional<domain::BusStat> GetBusStat(const std::string_view& bus_name) const;

    void Method() {
        domain::Stop stop = {"1", {12.32, 32.23332}};
    }

    // Возвращает маршруты, проходящие через
    std::optional<const std::unordered_set<domain::Bus*>*>
    GetBusesByStop(const std::string_view& stop_name) const;

    void RenderMap(std::ostream& output) const;

private:
    // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
    const t_c::TransportCatalogue& db_;
    const renderer::MapRenderer& renderer_;
};
