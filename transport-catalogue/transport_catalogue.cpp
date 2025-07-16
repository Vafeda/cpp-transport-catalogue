#include "transport_catalogue.h"

using namespace transport_catalogue;

void TransportCatalogue::SetStopStation(const std::string& id, const detail::Coordinates coordinates) {
	const StopStation* ptr_stop_station = GetStopStation(id);
	if (!ptr_stop_station) {
		stop_stations_.push_back({ id, coordinates });
		hash_table_stop_stations_[stop_stations_.back().name] = &stop_stations_.back();
	}
	else {
		const_cast<StopStation*>(ptr_stop_station)->coordinates = coordinates;
	}
}

void TransportCatalogue::SetBus(const std::string& id, const std::vector<std::string_view>& route) {
	bus_routes_.push_back({ id, {} });
	Bus& current_bus = bus_routes_.back();

	for (std::string_view stop : route) {
		auto it = hash_table_stop_stations_.find(stop);
		if (it == hash_table_stop_stations_.end()) {
			SetStopStation(std::string(stop), {});
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

	double length = 0.0;
	for (auto it = ptr_bus->route.begin() + 1; it != ptr_bus->route.end(); ++it) {
		length += ComputeDistance((*(it - 1))->coordinates, (*it)->coordinates);
	}

	route_info.stops_count = ptr_bus->route.size();
	route_info.unique_stops_count = unique_names.size();
	route_info.route_length = length;
	route_info.route_exists = true;

	return route_info;
}

const std::set<std::string_view>* TransportCatalogue::GetStopStationInfo(std::string_view id) const {
	static const std::set<std::string_view> empty_set;

	const StopStation* ptr_stop_station = GetStopStation(id);
	if (!ptr_stop_station) {
		return nullptr;
	}

	auto it_routes_in_stop = hash_table_routes_in_stop_.find(ptr_stop_station);
	if (it_routes_in_stop != hash_table_routes_in_stop_.end()) {
		return &(it_routes_in_stop->second);
	}
		
    return &empty_set;
}