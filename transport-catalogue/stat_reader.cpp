#include "stat_reader.h"

using namespace t_c;

namespace o_r {

void ProcessRequest(std::istream& input, std::ostream& output, 
                    t_c::TransportCatalogue& catalogue) {
    int stat_request_count;
    input >> stat_request_count >> std::ws;
    for (int i = 0; i < stat_request_count; ++i) {
        std::string line;
        getline(input, line);
        ParseAndPrintStat(catalogue, line, output);
    }
}

void ParseAndPrintStat(const TransportCatalogue& transport_catalogue,
                std::string_view request, std::ostream& output) {
    auto pos_first_letter = request.find_first_not_of(' ');
    auto pos_space = request.find(' ', pos_first_letter);
    std::string_view request_type = request.substr(pos_first_letter, pos_space - pos_first_letter);

    auto pos_begin_name = request.find_first_not_of(' ', pos_space);
    auto pos_end_name = request.find_last_not_of(' ');
    std::string_view name = request.substr(pos_begin_name, (pos_end_name - pos_begin_name) + 1);

    if (request_type == "Bus") {
        details::WriteBusRequest(output, name, transport_catalogue);
    } else if (request_type == "Stop") {
        details::WriteStopRequest(output, name, transport_catalogue);
    }
}

namespace details {
    void WriteBusRequest(std::ostream& output, std::string_view name
                    , const TransportCatalogue& catalogue) {
        const BusInfo binfo = catalogue.GetBusInfo(name);
        output << "Bus " << name << ": ";
        if (binfo.name.empty()) {
            output << "not found\n";
        } else {
            output << binfo.route_count << " stops on route, ";
            output << binfo.unique_count << " unique stops, ";
            output << std::setprecision(6) << binfo.length << " route length, ";
            output << std::setprecision(6) << binfo.curvature << " curvature\n";
        }
    }

    void WriteStopRequest(std::ostream& output, std::string_view name
                    , const TransportCatalogue& catalogue) {
        const Stop& stop = catalogue.FindStop(name);
        output << "Stop " << name << ": ";
        if (stop.name.empty()) {
            output << "not found\n";
        } else if (stop.buses.empty()) {
            output << "no buses\n";
        } else {
            bool is_first = true;
            output << "buses ";
            for (const auto& bus : stop.buses) {
                if (!is_first) {
                    output << " ";
                }
                is_first = false;
                output << bus;
            }
            output << "\n";
        }
    }
} // details

} // o_r