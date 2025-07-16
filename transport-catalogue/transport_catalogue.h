#pragma once
#include "geo.h"
#include <vector>
#include <deque>
#include <string>
#include <set>
#include <unordered_set>
#include <unordered_map>

namespace transport_catalogue {
	struct StopStation {
		std::string stop_station_name;
		detail::Coordinates coordinates;
	};

	struct Bus {
		std::string bus_name;
		std::vector<StopStation*> route;
	};

	struct RouteInfo {
		int stops_count;
		int unique_stops_count;
		double route_length;
		bool route_exists;
	};

	struct StopStationInfo {
		std::set<std::string> routes_in_stop;
		bool stop_station_exists;
	};

	class TransportCatalogue {
	public:
		void SetStopStation(std::string id, std::string description);
		void SetBus(std::string id, std::string description);

		StopStation* GetStopStation(std::string id) const;
		Bus* GetBus(std::string id) const;

		RouteInfo GetBusInfo(std::string id) const;
		StopStationInfo GetStopStationInfo(std::string id) const;

	private:
		std::deque<StopStation> stop_stations_;
		std::unordered_map<std::string, StopStation*> hash_table_stop_stations_;

		std::deque<Bus> bus_routes_;
		std::unordered_map<std::string, Bus*> hash_table_bus_routes_;

		std::unordered_map<StopStation*, std::set<std::string>> hash_table_routes_in_stop_;
	};
}