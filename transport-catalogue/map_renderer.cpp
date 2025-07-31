#include "map_renderer.h"

namespace map_renderer {
	svg::Document RenderMap::RenderAllLayers(const transport_catalogue::TransportCatalogue& tc) {
		svg::Document render_map;
		render_map.Merge(RenderBusRoutes());
		render_map.Merge(RenderBusLabels());
		render_map.Merge(RenderStopSymbols(tc));
		render_map.Merge(RenderStopLabels(tc));
		return render_map;
	}

	svg::Document RenderMap::RenderBusRoutes() {
		svg::Document render_map;

		size_t color_palette_index = 0;
		for (const auto& bus : buses_) {
			if (bus.route.empty()) {
				continue;
			}

			svg::Polyline p;
			for (const auto& stop : bus.route) {
				p.AddPoint(projector_(stop->coordinates));
			}

			render_map.Add(p.SetStrokeColor(rs_.color_palette_[color_palette_index++])
				.SetFillColor(svg::NoneColor)
				.SetStrokeWidth(rs_.line_width_)
				.SetStrokeLineCap(svg::StrokeLineCap::ROUND)
				.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND));
			color_palette_index %= rs_.color_palette_.size();
		}

		return render_map;
	}

	svg::Document RenderMap::RenderBusLabels() {
		svg::Document render_map;

		size_t color_palette_index = 0;
		for (const auto& bus : buses_) {
			if (bus.route.empty()) {
				continue;
			}

			svg::Text substrate_properties;
			substrate_properties
				.SetOffset(svg::Point(rs_.bus_label_offset_[0], rs_.bus_label_offset_[1]))
				.SetFontSize(rs_.bus_label_font_size_)
				.SetFontFamily("Verdana")
				.SetFontWeight("bold");

			svg::Text general_properties = substrate_properties;
			general_properties
				.SetFillColor(rs_.underlayer_color_)
				.SetStrokeColor(rs_.underlayer_color_)
				.SetStrokeWidth(rs_.underlayer_width_)
				.SetStrokeLineCap(svg::StrokeLineCap::ROUND)
				.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

			if (bus.is_roundtrip) {
				general_properties.SetPosition(projector_(bus.route.back()->coordinates))
					.SetData(bus.name);

				substrate_properties.SetPosition(projector_(bus.route.back()->coordinates))
					.SetData(bus.name)
					.SetFillColor(rs_.color_palette_[color_palette_index++]);
			}
			else {
				general_properties.SetPosition(projector_(bus.route.back()->coordinates))
					.SetData(bus.name);
				substrate_properties.SetPosition(projector_(bus.route.back()->coordinates))
					.SetData(bus.name)
					.SetFillColor(rs_.color_palette_[color_palette_index]);

				if ((bus.route[bus.route.size() / 2]->name != bus.route.back()->name)) {
					render_map.Add(general_properties);
					render_map.Add(substrate_properties);

					svg::Point g(projector_(bus.route[bus.route.size() / 2]->coordinates));
					general_properties.SetPosition(g)
						.SetData(bus.name);
					substrate_properties.SetPosition(g)
						.SetData(bus.name)
						.SetFillColor(rs_.color_palette_[color_palette_index]);
				}
				color_palette_index++;
			}

			render_map.Add(general_properties);
			render_map.Add(substrate_properties);
			color_palette_index %= rs_.color_palette_.size();
		}

		return render_map;
	}

	svg::Document RenderMap::RenderStopSymbols(const transport_catalogue::TransportCatalogue& tc) {
		svg::Document render_map;

		for (const auto& stop : stops_) {
			if (!tc.GetStopStationInfo(stop.name).empty()) {
				render_map.Add(svg::Circle().SetCenter(projector_(stop.coordinates))
					.SetRadius(rs_.stop_radius_)
					.SetFillColor("white"));
			}
		}

		return render_map;
	}

	svg::Document RenderMap::RenderStopLabels(const transport_catalogue::TransportCatalogue& tc) {
		svg::Document render_map;

		svg::Text substrate_properties;
		substrate_properties
			.SetOffset(svg::Point(rs_.stop_label_offset_[0], rs_.stop_label_offset_[1]))
			.SetFontSize(rs_.stop_label_font_size_)
			.SetFontFamily("Verdana");

		svg::Text general_properties = substrate_properties;
		general_properties
			.SetFillColor(rs_.underlayer_color_)
			.SetStrokeColor(rs_.underlayer_color_)
			.SetStrokeWidth(rs_.underlayer_width_)
			.SetStrokeLineCap(svg::StrokeLineCap::ROUND)
			.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

		for (const auto& stop : stops_) {
			if (!tc.GetStopStationInfo(stop.name).empty()) {
				general_properties.SetPosition(projector_(stop.coordinates))
					.SetData(stop.name);

				substrate_properties.SetPosition(projector_(stop.coordinates))
					.SetData(stop.name)
					.SetFillColor("black");
				render_map.Add(general_properties);
				render_map.Add(substrate_properties);
			}
		}

		return render_map;
	}

	SphereProjector RenderMap::CreateProjector(const std::deque<transport_catalogue::Bus>& buses, const RenderSettings& rs) const {
		std::vector<geo::Coordinates> coordinates;
		for (const auto& bus : buses) {
			for (const auto& stop : bus.route) {
				coordinates.push_back(stop->coordinates);
			}
		}

		return SphereProjector(coordinates.begin(), coordinates.end(), rs.width_, rs.height_, rs.padding_);
	}

	std::set<transport_catalogue::StopStation, RenderMap::StopStationCompare> RenderMap::CollectUniqueStops(
		const std::deque<transport_catalogue::Bus>& buses) const
	{
		std::set<transport_catalogue::StopStation, StopStationCompare> stops;
		for (const auto& bus : buses) {
			for (const auto& stop : bus.route) {
				stops.insert(*stop);
			}
		}
		return stops;
	}
}