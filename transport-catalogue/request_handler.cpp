#include "request_handler.h"

svg::Document RequestHandler::RenderMap() const {
	std::deque<transport_catalogue::Bus> deque_bus = tc_.GetAllRoute();

	std::sort(deque_bus.begin(), deque_bus.end(),
		[](const transport_catalogue::Bus& lhs, const transport_catalogue::Bus& rhs) {
			return lhs.name < rhs.name;
		}
	);

	map_renderer::RenderMap rm(rs_, deque_bus);

	return rm.RenderAllLayers(tc_);
}