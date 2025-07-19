#include "transport_catalogue.h"

using namespace transport_catalogue;

void TransportCatalogue::AddStopStation(const std::string& id, const detail::Coordinates coordinates) {
	const StopStation* ptr_stop_station = GetStopStation(id);
	if (!ptr_stop_station) {
		stop_stations_.push_back({ id, coordinates });
		hash_table_stop_stations_[stop_stations_.back().name] = &stop_stations_.back();
	}
	else {
		const_cast<StopStation*>(ptr_stop_station)->coordinates = coordinates;
	}
}

void TransportCatalogue::AddBus(const std::string& id, const std::vector<std::string_view>& route) {
	bus_routes_.push_back({ id, {} });
	Bus& current_bus = bus_routes_.back();

	for (std::string_view stop : route) {
		auto it = hash_table_stop_stations_.find(stop);
		if (it == hash_table_stop_stations_.end()) {
			AddStopStation(std::string(stop), {});
			it = hash_table_stop_stations_.find(stop);
		}

		hash_table_routes_in_stop_[it->second].insert(current_bus.name);

		current_bus.route.push_back(it->second);
	}

	hash_table_bus_routes_[current_bus.name] = &bus_routes_.back();
}

const StopStation* TransportCatalogue::GetStopStation(std::string_view id) const {
	auto it_stop_station = hash_table_stop_stations_.find(id);
	if (it_stop_station == hash_table_stop_stations_.end()) {
		return nullptr;
	}
	return it_stop_station->second;
}

const Bus* TransportCatalogue::GetBus(std::string_view id) const {
	auto it_bus = hash_table_bus_routes_.find(id);
	if (it_bus == hash_table_bus_routes_.end()) {
		return nullptr;
	}
	return it_bus->second;
}

std::optional<RouteInfo> TransportCatalogue::GetBusInfo(std::string_view id) const {
	const Bus* ptr_bus = GetBus(id);
	if (!ptr_bus) return std::nullopt;

	RouteInfo route_info;
	std::unordered_set<std::string_view> unique_names;
	for (const StopStation* stop : ptr_bus->route) {
		unique_names.insert(stop->name);
	}

	int length = 0;
	double curvature = 0;
	for (auto it = ptr_bus->route.begin() + 1; it != ptr_bus->route.end(); ++it) {
		std::optional<int> distance = GetDistanceBetweenStopsStations(*(it - 1), *it);
		if (distance.has_value()) {
			length += distance.value();
		}
		curvature += ComputeDistance((*(it - 1))->coordinates, (*it)->coordinates);
	}

	route_info.stops_count = ptr_bus->route.size();
	route_info.unique_stops_count = unique_names.size();
	route_info.route_length = length;
	route_info.curvature = length / curvature;
	route_info.route_exists = true;

	return route_info;
}

const std::set<std::string_view>& TransportCatalogue::GetStopStationInfo(std::string_view id) const {
	static const std::set<std::string_view> empty_set;

	const StopStation* ptr_stop_station = GetStopStation(id);
	if (!ptr_stop_station) {
		return empty_set;
	}

	auto it_routes_in_stop = hash_table_routes_in_stop_.find(ptr_stop_station);
	if (it_routes_in_stop != hash_table_routes_in_stop_.end()) {
		return it_routes_in_stop->second;
	}
		
    return empty_set;
}

void TransportCatalogue::SetDistanceBetweenStopsStations(std::string_view begin_stop_station, std::string_view end_stop_station, int distance) {
	static const double nan = std::nan("");

	const StopStation* ptr_begin_stop = GetStopStation(begin_stop_station);
	if (!ptr_begin_stop) {
		AddStopStation(std::string(begin_stop_station), { nan, nan });
		ptr_begin_stop = GetStopStation(begin_stop_station);
	}

	const StopStation* ptr_end_stop = GetStopStation(end_stop_station);
	if (!ptr_end_stop) {
		AddStopStation(std::string(end_stop_station), { nan, nan });
		ptr_end_stop = GetStopStation(end_stop_station);
	}

	hash_table_distance_between_stops[{ptr_begin_stop, ptr_end_stop}] = distance;
}

std::optional<int> TransportCatalogue::GetDistanceBetweenStopsStations(const StopStation* begin_stop_station, const StopStation* end_stop_station) const {
	auto it_distance = hash_table_distance_between_stops.find({ begin_stop_station, end_stop_station });
	if (it_distance == hash_table_distance_between_stops.end()) {
		it_distance = hash_table_distance_between_stops.find({ end_stop_station, begin_stop_station });
		if (it_distance == hash_table_distance_between_stops.end()) {
			return std::nullopt;
		}
		return it_distance->second;
	}

	return it_distance->second;
}