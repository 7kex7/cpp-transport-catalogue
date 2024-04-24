#pragma once
#include <algorithm>
#include <deque>
#include <cassert>
#include <iostream>
#include <stdexcept>
#include <string>
#include <sstream>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <set>

#include "geo.h"
#include "json.h"
#include "json_builder.h"
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

    void ProcessBaseRequests(const json::Array&);
    void ProcessRenderRequests(const json::Dict&
                                    , renderer::RenderSettings&);

    using RoadDistances = const std::unordered_map<std::string, double>;
private:
    std::deque<StopData> stop_requests_;
    std::deque<BusData> bus_requests_;
    t_c::TransportCatalogue& db_;

    void LoadBusReq(const json::Dict&);
    void LoadStopReq(const json::Dict&);
    
    void ApplyDistances(domain::Stop&, RoadDistances&);
};


class StatRequests : public RequestHandler {
public:
    StatRequests(const t_c::TransportCatalogue&
                    , const renderer::MapRenderer&, json::Builder&);
    void PrintJsonDocument(const json::Array&, std::ostream&);

private:
    json::Builder& builder_;
    void HandleBusRequest(const json::Dict&);
    void HandleStopRequest(const json::Dict&);
    void HandleMapRequest(const json::Dict&);
};

void LoadJSON(std::istream&, std::ostream&, t_c::TransportCatalogue&);

} // json_reader
