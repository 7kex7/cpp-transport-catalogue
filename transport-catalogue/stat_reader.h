#pragma once

#include <iomanip>
#include <iosfwd>
#include <string_view>

#include "transport_catalogue.h"

namespace o_r {

    void ParseAndPrintStat(const t_c::TransportCatalogue& transport_catalogue, std::string_view request,
                        std::ostream& output);

} // o_r