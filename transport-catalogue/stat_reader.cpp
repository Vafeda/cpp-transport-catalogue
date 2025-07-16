#include "stat_reader.h"
#include <iostream>
#include <iomanip>

void transport_catalogue::stat_reader::ParseAndPrintStat(const transport_catalogue::TransportCatalogue& transport_catalogue, std::string_view request,
    std::ostream& output) {
    size_t middle_pos = request.find_first_of(" ");

    if (middle_pos == std::string::npos) {
        throw std::invalid_argument("Не правильный запрос");
    }

    std::string command = std::string(request.substr(0, middle_pos));
    std::string key = std::string(request.substr(middle_pos + 1));

    if (command == "Stop") {
        transport_catalogue::StopStationInfo info = transport_catalogue.GetStopStationInfo(key);
        if (!info.stop_station_exists) {
            output << request.data() << ": not found" << std::endl;
        }
        else if (info.routes_in_stop.empty()) {
            output << request.data() << ": no buses" << std::endl;
        }
        else {
            output << request.data() << ": buses";
            for (std::string route : info.routes_in_stop) {
                output << " " << route;
            }
            output << std::endl;
        }

    }
    else if (command == "Bus") {
        transport_catalogue::RouteInfo info = transport_catalogue.GetBusInfo(key);

        if (!info.route_exists) {
            output << request.data() << ": not found" << std::endl;
        }
        else {
            output << std::setprecision(6) << request.data() << ": "
                << info.stops_count << " stops on route, "
                << info.unique_stops_count << " unique stops, "
                << info.route_length << " route length" << std::endl;
        }
    }
}