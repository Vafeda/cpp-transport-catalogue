#include "request_handler.h"

svg::Document RequestHandler::RenderMap() const {

	std::deque<transport_catalogue::Bus> deque_bus = tc_.GetAllRoute();

	std::sort(deque_bus.begin(), deque_bus.end(),
		[](const transport_catalogue::Bus& lhs, const transport_catalogue::Bus& rhs) {
			return lhs.name < rhs.name;
		}
	);

	std::vector<geo::Coordinates> coordinates;
	for (auto& bus : deque_bus) {
		for (const auto& stop : bus.route) {
			coordinates.push_back(stop->coordinates);
		}
	}

	const map_renderer::SphereProjector projector(coordinates.begin(), coordinates.end(),
												  rs_.width_, rs_.height_,
												rs_.padding_);
	
	map_renderer::RenderMap rm(rs_, projector);

	svg::Document render_map;
	rm.RenderBusRoutes(deque_bus, render_map);
	rm.RenderBusLabels(deque_bus, render_map);

	{
		map_renderer::RenderMap rm(rs_, projector);

		std::deque<transport_catalogue::StopStation> deque_stop = tc_.GetAllStopStation();

		std::sort(deque_stop.begin(), deque_stop.end(),
			[](const transport_catalogue::StopStation& lhs, const transport_catalogue::StopStation& rhs) {
				return lhs.name < rhs.name;
			}
		);

		rm.RenderStopSymbols(deque_stop, tc_, render_map);
		rm.RenderStopLabels(deque_stop, tc_, render_map);
	}

	return render_map;
}