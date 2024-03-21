#pragma once
#include <algorithm>
#include <cassert>
#include <deque>
#include <iostream>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_set>
#include <unordered_map>

#include "geo.h"
#include "json.h"
#include "domain.h"
#include "transport_catalogue.h"
#include "request_handler.h"

namespace json_reader {

class FillRequests {
    struct StopData {
        StopData() = delete;
        StopData(std::string nm, double lat, double lng) 
            : name(std::move(nm)), coords({lat, lng}) {
        }
        std::string name;
        geo::Coordinates coords;
        std::unordered_map<std::string, double> road_distances;
    };

    struct BusData {
        BusData() = delete;
        BusData(std::string nm, std::deque<std::string> st, bool is_round) 
            : name(std::move(nm)), stops(st), is_roundtrip(is_round) {}
        std::string name;
        std::deque<std::string> stops;
        bool is_roundtrip;
    };

public:
    FillRequests(t_c::TransportCatalogue& db);

    void ProcessBaseRequests(const json::Array& arr_reqs);
    void ProcessRenderRequests(const json::Dict& render_settings
                                    , renderer::RenderSettings& settings);

    using RoadDistances = const std::unordered_map<std::string, double>;
private:
    std::deque<StopData> stop_requests_;
    std::deque<BusData> bus_requests_;
    t_c::TransportCatalogue& db_;

    void LoadBusReq(const json::Dict& req);
    void LoadStopReq(const json::Dict& req);
    
    void ApplyDistances(domain::Stop& from_stop , RoadDistances& road_distances);
};


class StatRequests : public RequestHandler {
public:
    StatRequests(const t_c::TransportCatalogue& db
                    , const renderer::MapRenderer& renderer);
    void PrintJsonDocument(const json::Array& arr_reqs, std::ostream& output);

private:
    json::Node HandleBusRequest(const json::Dict& req);
    json::Node HandleStopRequest(const json::Dict& req);
    json::Node HandleMapRequest(const json::Dict& req);
};

void LoadJSON(std::istream& input, std::ostream& output, t_c::TransportCatalogue& catalogue);

} // json_reader
