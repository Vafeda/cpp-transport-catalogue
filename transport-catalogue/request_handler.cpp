#include "request_handler.h"

svg::Document RequestHandler::RenderMap() const {
	std::deque<transport_catalogue::Bus> deque_bus = db_.GetAllRoute();

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

	map_renderer::SphereProjector projector(coordinates.begin(), coordinates.end(),
											rs_.GetWidth(), rs_.GetHeight(),
											rs_.GetPadding());

	svg::Document render_map;
	
	// Drawing lines
	size_t color_palette_index = 0;
	for (auto& bus : deque_bus) {
		if (bus.route.empty()) {
			continue;
		}

		svg::Polyline p;
		for (const auto& stop : bus.route) {
			p.AddPoint(projector(stop->coordinates));
		}

		render_map.Add(p.SetStrokeColor(rs_.GetColorPalette()[color_palette_index++])
						.SetFillColor(svg::NoneColor)
						.SetStrokeWidth(rs_.GetLineWidth())
						.SetStrokeLineCap(svg::StrokeLineCap::ROUND)
						.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND));
		color_palette_index %= rs_.GetColorPalette().size();
	}

	// Drawing route names
	color_palette_index = 0;
	for (auto& bus : deque_bus) {
		if (bus.route.empty()) {
			continue;
		}

		svg::Text substrate_properties;
		substrate_properties
			.SetOffset(svg::Point(rs_.GetBusLabelOffset()[0], rs_.GetBusLabelOffset()[1]))
			.SetFontSize(rs_.GetBusLabelFontSize())
			.SetFontFamily("Verdana")
			.SetFontWeight("bold");

		svg::Text general_properties = substrate_properties;
		general_properties
			.SetFillColor(rs_.GetUnderlayerColor())
			.SetStrokeColor(rs_.GetUnderlayerColor())
			.SetStrokeWidth(rs_.GetUnderlayerWidth())
			.SetStrokeLineCap(svg::StrokeLineCap::ROUND)
			.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

		if (bus.is_roundtrip) {
			general_properties.SetPosition(projector(bus.route.back()->coordinates))
							.SetData(bus.name);

			substrate_properties.SetPosition(projector(bus.route.back()->coordinates))
							.SetData(bus.name)
							.SetFillColor(rs_.GetColorPalette()[color_palette_index++]);
		}
		else {
			general_properties.SetPosition(projector(bus.route.back()->coordinates))
				.SetData(bus.name);
			substrate_properties.SetPosition(projector(bus.route.back()->coordinates))
				.SetData(bus.name)
				.SetFillColor(rs_.GetColorPalette()[color_palette_index]);

			if ((bus.route[bus.route.size() / 2]->name != bus.route.back()->name)) {
				render_map.Add(general_properties);
				render_map.Add(substrate_properties);

				svg::Point g(projector(bus.route[bus.route.size() / 2]->coordinates));
				general_properties.SetPosition(g)
					.SetData(bus.name);
				substrate_properties.SetPosition(g)
					.SetData(bus.name)
					.SetFillColor(rs_.GetColorPalette()[color_palette_index]);
			}
			color_palette_index++;
		}

		render_map.Add(general_properties);
		render_map.Add(substrate_properties);
		color_palette_index %= rs_.GetColorPalette().size();
	}

	// Drawing symbol stop
	std::deque<transport_catalogue::StopStation> deque_stop = db_.GetAllStopStation();

	std::sort(deque_stop.begin(), deque_stop.end(),
		[](const transport_catalogue::StopStation& lhs, const transport_catalogue::StopStation& rhs) {
			return lhs.name < rhs.name;
		}
	);

	for (auto& stop : deque_stop) {
		if (!db_.GetStopStationInfo(stop.name).empty()) {
			render_map.Add(svg::Circle().SetCenter(projector(stop.coordinates))
				.SetRadius(rs_.GetStopRadius())
				.SetFillColor("white"));
		}
	}

	// Drawing text stop
	{
		svg::Text substrate_properties;
		substrate_properties
			.SetOffset(svg::Point(rs_.GetStopLabelOffset()[0], rs_.GetStopLabelOffset()[1]))
			.SetFontSize(rs_.GetStopLabelFontSize())
			.SetFontFamily("Verdana");

		svg::Text general_properties = substrate_properties;
		general_properties
			.SetFillColor(rs_.GetUnderlayerColor())
			.SetStrokeColor(rs_.GetUnderlayerColor())
			.SetStrokeWidth(rs_.GetUnderlayerWidth())
			.SetStrokeLineCap(svg::StrokeLineCap::ROUND)
			.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

		for (auto& stop : deque_stop) {
			if (!db_.GetStopStationInfo(stop.name).empty()) {
				general_properties.SetPosition(projector(stop.coordinates))
					.SetData(stop.name);

				substrate_properties.SetPosition(projector(stop.coordinates))
					.SetData(stop.name)
					.SetFillColor("black");
				render_map.Add(general_properties);
				render_map.Add(substrate_properties);
			}
		}
	}
	return render_map;
}