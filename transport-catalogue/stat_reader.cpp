#include "stat_reader.h"
#include <iostream>
#include <iomanip>

using namespace transport_catalogue;

void stat_reader::ParseAndPrintStat(const TransportCatalogue& transport_catalogue, std::string_view request,
    std::ostream& output) {
    size_t middle_pos = request.find_first_of(" ");

    if (middle_pos == std::string::npos) {
        throw std::invalid_argument("Не правильный запрос");
    }

    std::string command = std::string(request.substr(0, middle_pos));
    std::string key = std::string(request.substr(middle_pos + 1));

    if (command == "Stop") {
        stat_reader::PrintStopStat(transport_catalogue.GetStopStationInfo(key), request, output);
    }
    else if (command == "Bus") {
        stat_reader::PrintBusStat(transport_catalogue.GetBusInfo(key), request, output);
    }
}

void stat_reader::PrintStopStat(std::optional<std::set<std::string_view>> set_of_route_in_stop, std::string_view request,
    std::ostream& output) {
    if (!set_of_route_in_stop) {
        output << request.data() << ": not found" << std::endl;
    }
    else if (set_of_route_in_stop.value().empty()) {
        output << request.data() << ": no buses" << std::endl;
    }
    else {
        output << request.data() << ": buses";
        for (std::string_view route : set_of_route_in_stop.value()) {
            output << " " << route.data();
        }
        output << std::endl;
    }
}

void stat_reader::PrintBusStat(std::optional<RouteInfo> bus_info, std::string_view request,
    std::ostream& output) {
    if (!bus_info) {
        output << request.data() << ": not found" << std::endl;
    }
    else {
        output << std::setprecision(6) << request.data() << ": "
            << bus_info->stops_count << " stops on route, "
            << bus_info->unique_stops_count << " unique stops, "
            << bus_info->route_length << " route length" << std::endl;
    }
}