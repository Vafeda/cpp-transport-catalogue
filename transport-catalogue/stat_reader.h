#pragma once

#include <iosfwd>
#include <string_view>

#include "transport_catalogue.h"

namespace transport_catalogue {
    namespace stat_reader {
        void ParseAndPrintStat(const TransportCatalogue& transport_catalogue, std::string_view request,
            std::ostream& output);

        void PrintStopStat(std::optional<std::set<std::string_view>> set_of_route_in_stop, std::string_view request,
            std::ostream& output);

        void PrintBusStat(std::optional<RouteInfo> bus_info, std::string_view request,
            std::ostream& output);
    }
}