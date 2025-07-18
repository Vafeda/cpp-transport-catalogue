#pragma once
#include "geo.h"
#include <deque>
#include <optional>
#include <set>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace transport_catalogue {
	struct StopStation {
		std::string name;
		detail::Coordinates coordinates;
	};

	struct Bus {
		std::string name;
		std::vector<const StopStation*> route;
	};

	struct RouteInfo {
		size_t stops_count;
		size_t unique_stops_count;
		int route_length;
		double curvature;
		bool route_exists;
	};

	class TransportCatalogue {
	public:
		void SetStopStation(const std::string& id, const detail::Coordinates coordinates);
		void SetBus(const std::string& id, const std::vector<std::string_view>& route);
		void SetDistanceBetweenStopsStations(const std::string& begin_stop_station, const std::string& end_stop_station, int distance);

		const StopStation* GetStopStation(std::string_view id) const;
		const Bus* GetBus(std::string_view id) const;
		int GetDistanceBetweenStopsStations(const StopStation* begin_stop_station, const StopStation* end_stop_station) const;

		std::optional<RouteInfo> GetBusInfo(std::string_view id) const;
		const std::set<std::string_view>& GetStopStationInfo(std::string_view id) const;
	
	private:
		struct StopPairHash {
			size_t operator()(const std::pair<const StopStation*, const StopStation*>& stop_pair) const {
				auto h1 = std::hash<const void*>{}(static_cast<const void*>(stop_pair.first));
				auto h2 = std::hash<const void*>{}(static_cast<const void*>(stop_pair.second));

				return h1 ^ (h2 << 1);
			}
		};

	private:
		std::deque<StopStation> stop_stations_;
		std::unordered_map<std::string_view, const StopStation*> hash_table_stop_stations_;

		std::deque<Bus> bus_routes_;
		std::unordered_map<std::string_view, const Bus*> hash_table_bus_routes_;

		std::unordered_map<const StopStation*, std::set<std::string_view>> hash_table_routes_in_stop_;

		std::unordered_map<std::pair<const StopStation*, const StopStation*>, int, StopPairHash> hash_table_distance_between_stops;
	};
}