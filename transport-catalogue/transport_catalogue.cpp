#include "transport_catalogue.h"
#include "input_reader.h"

using namespace transport_catalogue;

void transport_catalogue::TransportCatalogue::SetStopStation(std::string id, std::string description) {
	auto key_stop_station = hash_table_stop_stations_.find(id);
	if (key_stop_station == hash_table_stop_stations_.end()) {
		stop_stations_.push_back({ std::move(id), input_reader::ParseCoordinates(description) });
		hash_table_stop_stations_[stop_stations_.back().stop_station_name] = &stop_stations_.back();
	}
	else {
		key_stop_station->second->coordinates = input_reader::ParseCoordinates(description);
	}
}

void transport_catalogue::TransportCatalogue::SetBus(std::string id, std::string description) {
	std::vector<std::string_view> route = input_reader::ParseRoute(description);
	Bus new_bus{ std::move(id), {} };
	for (std::string_view stop : route) {
		auto it = hash_table_stop_stations_.find(std::string(stop));
		if (it == hash_table_stop_stations_.end()) {
			SetStopStation(std::string(stop), "");
			it = hash_table_stop_stations_.find(std::string(stop));
		}

		hash_table_routes_in_stop_[it->second].insert(new_bus.bus_name);

		new_bus.route.push_back(it->second);
	}
	bus_routes_.push_back(new_bus);
	hash_table_bus_routes_.insert({ new_bus.bus_name, &bus_routes_.back() });
}

transport_catalogue::StopStation* transport_catalogue::TransportCatalogue::GetStopStation(std::string id) const {
	auto it_stop_station = hash_table_stop_stations_.find(id);
	if (it_stop_station == hash_table_stop_stations_.end()) {
		return nullptr;
	}
	return it_stop_station->second;
}

transport_catalogue::Bus* transport_catalogue::TransportCatalogue::GetBus(std::string id) const {
	auto it_bus = hash_table_bus_routes_.find(id);
	if (it_bus == hash_table_bus_routes_.end()) {
		return nullptr;
	}
	return it_bus->second;
}

transport_catalogue::RouteInfo transport_catalogue::TransportCatalogue::GetBusInfo(std::string id) const {
	RouteInfo route_info{};

	auto it_bus = hash_table_bus_routes_.find(id);
	if (it_bus == hash_table_bus_routes_.end()) {
		route_info.route_exists = false;
		return route_info;
	}

	// Берем указатель на структуру Bus, чтоб дальше не писать (it_bus->second)
	const Bus* bus_ptr = it_bus->second;

	std::unordered_set<std::string_view> unique_names;

	for (const StopStation* stop : bus_ptr->route) {
		unique_names.insert(stop->stop_station_name);
	}

	double length = 0.0;
	for (auto it_route = bus_ptr->route.begin() + 1; it_route != bus_ptr->route.end(); ++it_route) {
		const StopStation* prev_stop = *(it_route - 1);
		const StopStation* curr_stop = *it_route;

		length += ComputeDistance(
			prev_stop->coordinates,
			curr_stop->coordinates
		);
	}

	route_info.stops_count = static_cast<int>(bus_ptr->route.size());
	route_info.unique_stops_count = static_cast<int>(unique_names.size());
	route_info.route_length = length;
	route_info.route_exists = true;

	return route_info;
}

transport_catalogue::StopStationInfo transport_catalogue::TransportCatalogue::GetStopStationInfo(std::string id) const {
	StopStationInfo stop_station_info{};
	auto it_stop_station = hash_table_stop_stations_.find(id);
	if (it_stop_station == hash_table_stop_stations_.end()) {
		stop_station_info.stop_station_exists = false;
		return stop_station_info;
	}

	auto it_routes_in_stop = hash_table_routes_in_stop_.find(it_stop_station->second);
	if (it_routes_in_stop != hash_table_routes_in_stop_.end()) {
		stop_station_info.routes_in_stop = it_routes_in_stop->second;
	}

	stop_station_info.stop_station_exists = true;
	return stop_station_info;
}