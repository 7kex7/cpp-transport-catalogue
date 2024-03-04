#pragma once

#include <iomanip>
#include <iostream>
#include <iosfwd>
#include <string_view>

#include "transport_catalogue.h"

namespace o_r {
/**
 * Точка выхода
*/
void ProcessRequest(std::istream& input, std::ostream& output, t_c::TransportCatalogue& catalogue);

void ParseAndPrintStat(const t_c::TransportCatalogue& transport_catalogue, std::string_view request,
                    std::ostream& output);

namespace details {
    void WriteBusRequest(std::ostream& output, std::string_view name, const t_c::TransportCatalogue& catalogue);
    void WriteStopRequest(std::ostream& output, std::string_view name, const t_c::TransportCatalogue& catalogue);
}

} // o_r