#pragma once
#include "geo.h"
#include <deque>
#include <optional>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
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
		double route_length;
		bool route_exists;
	};

	class TransportCatalogue {
	public:
		void SetStopStation(const std::string& id, const detail::Coordinates coordinates);
		void SetBus(const std::string& id, const std::vector<std::string_view>& route);

		const StopStation* GetStopStation(std::string_view id) const;
		const Bus* GetBus(std::string_view id) const;

		std::optional<RouteInfo> GetBusInfo(std::string_view id) const;
		const std::set<std::string_view>& GetStopStationInfo(std::string_view id) const;

	private:
		std::deque<StopStation> stop_stations_;
		std::unordered_map<std::string_view, const StopStation*> hash_table_stop_stations_;

		std::deque<Bus> bus_routes_;
		std::unordered_map<std::string_view, const Bus*> hash_table_bus_routes_;

		std::unordered_map<const StopStation*, std::set<std::string_view>> hash_table_routes_in_stop_;
	};
}