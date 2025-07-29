#include "json_reader.h"

namespace json_reader {

	//-------------------------------------------------------------------
	//---------------Function Apply Transport Catalogue------------------
	//-------------------------------------------------------------------
	inline std::string error_messeg_base_requests_type = "JsonReader(ApplyBaseRequests): Missing required field 'type' in request"s;
	inline std::string error_messeg_base_requests_stop = "JsonReader(ApplyStopInfo): The dictionary that describes \"Stop\" is missing the key "s;
	inline std::string error_messeg_base_requests_bus = "JsonReader(ApplyBusInfo): The dictionary that describes \"Bus\" is missing the key "s;

	transport_catalogue::TransportCatalogue JsonReader::ApplyBaseRequests() const {
		auto base_requests = json_.find(base_key);
		if (base_requests == json_.end()) {
			throw std::logic_error("The dictionary is missing a key\"" + base_key + "\"");
		}

		transport_catalogue::TransportCatalogue catalogue;

		try {
			const json::Array array = base_requests->second.AsArray();

			for (const json::Node& a : array) {
				json::Dict map;

				map = a.AsMap();

				auto it_type = map.find("type");
				if (it_type == map.end()) {
					throw std::logic_error(error_messeg_base_requests_type);
				}

				if (it_type->second.AsString() == "Stop") {
					ApplyStopInfo(map, catalogue);
				}

			}

			for (const json::Node& a : array) {
				json::Dict map;

				map = a.AsMap();

				auto it_type = map.find("type");
				if (it_type == map.end()) {
					throw std::logic_error(error_messeg_base_requests_type);
				}

				if (it_type->second.AsString() == "Bus") {
					ApplyBusInfo(map, catalogue);
				}
			}

		}
		catch (const std::exception& e) {
			throw;
		}
	
		return catalogue;
	}
	
	void JsonReader::ApplyStopInfo(const json::Dict& dict, transport_catalogue::TransportCatalogue& tc) const {
		auto it_name = dict.find("name");
		if (it_name == dict.end()) {
			throw std::logic_error(error_messeg_base_requests_stop + "\"name\"");
		}

		// Apply StopStation (for success need to know "name", "latitude", "longitude")
		{
			auto it_lat = dict.find("latitude");
			if (it_lat == dict.end()) {
				throw std::logic_error(error_messeg_base_requests_stop + "\"latitude\"");
			}

			auto it_lon = dict.find("longitude");
			if (it_lon == dict.end()) {
				throw std::logic_error(error_messeg_base_requests_stop + "\"longitude\"");
			}

			try {
				tc.AddStopStation(it_name->second.AsString(), { it_lat->second.AsDouble(), it_lon->second.AsDouble() });
			}
			catch (const std::exception& e) {
				throw;
			}
		}

		// Apply road distance between StopStations (for success need to know "name", std::map<"name", int>)
		{
			auto it_road_distances = dict.find("road_distances");
			if (it_road_distances == dict.end()) {
				throw std::logic_error(error_messeg_base_requests_stop + "\"road_distances\"");
			}

			try {
				const json::Dict road_distances = it_road_distances->second.AsMap();
				for (const auto& [key, value] : road_distances) {
					tc.SetDistanceBetweenStopsStations(it_name->second.AsString(), key, value.AsInt());
				}
			}
			catch (const std::exception& e) {
				throw;
			}
		}
	}

	void JsonReader::ApplyBusInfo(const json::Dict& dict, transport_catalogue::TransportCatalogue& tc) const {
		auto it_name = dict.find("name");
		if (it_name == dict.end()) {
			throw std::logic_error(error_messeg_base_requests_bus + "\"name\"");
		}

		auto it_stops = dict.find("stops");
		if (it_stops == dict.end()) {
			throw std::logic_error(error_messeg_base_requests_bus + "\"stops\"");
		}

		auto it_is_roundtrip = dict.find("is_roundtrip");
		if (it_is_roundtrip == dict.end()) {
			throw std::logic_error(error_messeg_base_requests_bus + "\"is_roundtrip\"");
		}

		// Apply bus route (for success need to know "name", "stops", "is_roundtrip")
		try {
			const json::Array& stops = it_stops->second.AsArray();
			std::vector<std::string_view> route;
			for (const json::Node& stop : stops) {
				route.push_back(stop.AsString());
			}

			if (!it_is_roundtrip->second.AsBool()) {
				route.insert(route.end(), std::next(route.rbegin()), route.rend());
			}

			tc.AddBus(it_name->second.AsString(), route, it_is_roundtrip->second.AsBool());
		}
		catch (const std::exception& e) {
			throw;
		}
	}

	//-------------------------------------------------------------------
	//----------------Function Apply Rendering Settings------------------
	//-------------------------------------------------------------------
	inline std::string error_messeg_render_setting = "JsonReader(ApplyRenderSetting): The \"render_settings\" dictionary missing the key "s;
	
	map_renderer::RenderSettings JsonReader::ApplyRenderSettings() const {
		auto render_settings = json_.find(render_key);
		if (render_settings == json_.end()) {
			throw std::logic_error("JsonReader(ApplyRenderSetting): The dictionary is missing a key\"" + render_key + "\"");
		}

		try {
			const json::Dict dict = render_settings->second.AsMap();
			
			map_renderer::RenderSettings rs;
			ApplyWidthHeightPadding(dict, rs);
			ApplyLineWidthStopRadius(dict, rs);
			ApplyBusLabel(dict, rs);
			ApplyStopLabel(dict, rs);
			ApplyUnderlayer(dict, rs);
			ApplyColorPalette(dict, rs);
			return rs;
		}
		catch (const std::logic_error& e) {
			throw e;
		}
	}

	void JsonReader::ApplyWidthHeightPadding(const json::Dict& dict, map_renderer::RenderSettings& rs) const {
		try {
			auto it_width = dict.find("width"s);
			if (it_width == dict.end()) {
				throw std::logic_error(error_messeg_render_setting + "\"width\""s);
			}
			rs.SetWidth(it_width->second.AsDouble());

			auto it_height = dict.find("height"s);
			if (it_height == dict.end()) {
				throw std::logic_error(error_messeg_render_setting + "\"height\""s);
			}
			rs.SetHeight(it_height->second.AsDouble());

			auto it_padding = dict.find("padding"s);
			if (it_padding == dict.end()) {
				throw std::logic_error(error_messeg_render_setting + "\"padding\"");
			}
			rs.SetPadding(it_padding->second.AsDouble());

		}
		catch (const std::exception& e) {
			throw;
		}
	}

	void JsonReader::ApplyLineWidthStopRadius(const json::Dict& dict, map_renderer::RenderSettings& rs) const {
		try {
			auto it_line_width = dict.find("line_width"s);
			if (it_line_width == dict.end()) {
				throw std::logic_error(error_messeg_render_setting + "\"line_width\""s);
			}
			rs.SetLineWidth(it_line_width->second.AsDouble());

			auto it_stop_radius = dict.find("stop_radius"s);
			if (it_stop_radius == dict.end()) {
				throw std::logic_error(error_messeg_render_setting + "\"stop_radius\""s);
			}
			rs.SetStopRadius(it_stop_radius->second.AsDouble());
		}
		catch (const std::exception& e) {
			throw;
		}
	}

	void JsonReader::ApplyBusLabel(const json::Dict& dict, map_renderer::RenderSettings& rs) const {
		try {
			auto it_bus_label_font_size = dict.find("bus_label_font_size"s);
			if (it_bus_label_font_size == dict.end()) {
				throw std::logic_error(error_messeg_render_setting + "\"bus_label_font_size\""s);
			}
			rs.SetBusLabelFontSize(it_bus_label_font_size->second.AsInt());

			auto it_bus_label_offset = dict.find("bus_label_offset"s);
			if (it_bus_label_offset == dict.end()) {
				throw std::logic_error(error_messeg_render_setting + "\"bus_label_offset\""s);
			}
			
			json::Array array = it_bus_label_offset->second.AsArray();
			std::vector<double> result;
			for (auto& node : array) {
				result.push_back(node.AsDouble());
			}
			rs.SetBusLabelOffset(result);
		}
		catch (const std::exception& e) {
			throw e;
		}
	}

	void JsonReader::ApplyStopLabel(const json::Dict& dict, map_renderer::RenderSettings& rs) const {
		try {
			auto it_bus_label_font_size = dict.find("stop_label_font_size"s);
			if (it_bus_label_font_size == dict.end()) {
				throw std::logic_error(error_messeg_render_setting + "\"stop_label_font_size\""s);
			}
			rs.SetStopLabelFontSize(it_bus_label_font_size->second.AsInt());

			auto it_bus_label_offset = dict.find("stop_label_offset"s);
			if (it_bus_label_offset == dict.end()) {
				throw std::logic_error(error_messeg_render_setting + "\"stop_label_offset\""s);
			}

			json::Array array = it_bus_label_offset->second.AsArray();
			std::vector<double> result;
			for (auto& node : array) {
				result.push_back(node.AsDouble());
			}
			rs.SetStopLabelOffset(result);
		}
		catch (const std::exception& e) {
			throw e;
		}
	}

	void JsonReader::ApplyUnderlayer(const json::Dict& dict, map_renderer::RenderSettings& rs) const {
		try {
			auto it_underlayer_color = dict.find("underlayer_color"s);
			if (it_underlayer_color == dict.end()) {
				throw std::logic_error(error_messeg_render_setting + "\"underlayer_color\""s);
			}
			
			svg::Color result;

			if (it_underlayer_color->second.IsString()) {
				result = it_underlayer_color->second.AsString();
			}
			else if (it_underlayer_color->second.IsArray()) {
				json::Array color = it_underlayer_color->second.AsArray();
				if (color.size() == 3) {
					result = svg::Rgb{ static_cast<uint8_t>(color[0].AsInt()),
									   static_cast<uint8_t>(color[1].AsInt()), 
									   static_cast<uint8_t>(color[2].AsInt()) };
				}
				else if (color.size() == 4) {
					result = svg::Rgba{ static_cast<uint8_t>(color[0].AsInt()), 
										static_cast<uint8_t>(color[1].AsInt()), 
										static_cast<uint8_t>(color[2].AsInt()), 
										color[3].AsDouble() };
				}
				else {
					throw std::logic_error("JsonReader(ApplyUnderlayer): Invalid array of color descriptions");
				}
			}
			else {
				throw std::invalid_argument("JsonReader(ApplyUnderlayer): Incorrect color specification");
			}

			rs.SetUnderlayerColor(result);

			auto it_underlayer_width = dict.find("underlayer_width"s);
			if (it_underlayer_width == dict.end()) {
				throw std::logic_error(error_messeg_render_setting + "\"underlayer_width\""s);
			}

			rs.SetUnderlayerWidth(it_underlayer_width->second.AsDouble());
		}
		catch (const std::exception& e) {
			throw e;
		}
	}

	void JsonReader::ApplyColorPalette(const json::Dict& dict, map_renderer::RenderSettings& rs) const {
		try {
			auto it_bus_label_offset = dict.find("color_palette"s);
			if (it_bus_label_offset == dict.end()) {
				throw std::logic_error(error_messeg_render_setting + "\"color_palette\""s);
			}

			json::Array array = it_bus_label_offset->second.AsArray();
			std::vector<svg::Color> result;
			for (auto& node : array) {
				if (node.IsString()) {
					result.push_back(node.AsString());
				}
				else if (node.IsArray()) {
					json::Array color = node.AsArray();
					if (color.size() == 3) {
						result.push_back(svg::Rgb{ static_cast<uint8_t>(color[0].AsInt()),
													static_cast<uint8_t>(color[1].AsInt()),
													static_cast<uint8_t>(color[2].AsInt()) });
					}
					else if (color.size() == 4) {
						result.push_back(svg::Rgba{ static_cast<uint8_t>(color[0].AsInt()),
										static_cast<uint8_t>(color[1].AsInt()),
										static_cast<uint8_t>(color[2].AsInt()),
										color[3].AsDouble() });
					}
					else {
						throw std::invalid_argument("JsonReader(ApplyColorPalette): Incorrect color specification");
					}
				}
				else {
					throw std::invalid_argument("JsonReader(ApplyColorPalette): Incorrect color specification");
				}
			}
			rs.SetColorPalette(result);
		}
		catch (const std::exception& e) {
			throw e;
		}
	}

	//-------------------------------------------------------------------
	//----------------------Function Stat Info---------------------------
	//-------------------------------------------------------------------
	const json::Document JsonReader::StatInfo(const transport_catalogue::TransportCatalogue& catalogue, const RequestHandler& rh) const {
		auto stat_requests = json_.find(stat_key);
		if (stat_requests == json_.end()) {
			throw std::logic_error("The dictionary is missing a key\"" + stat_key + "\"");
		}

		const json::Array array = stat_requests->second.AsArray();

		json::Array gl;
		for (const json::Node& a : array) {
			json::Dict map;
			try {
				map = a.AsMap();
			}
			catch (const std::exception& e) {
				throw;
			}


			auto it_id = map.find("id");
			if (it_id == map.end()) {
				throw;
			}

			auto it_type = map.find("type");
			if (it_type == map.end()) {
				throw;
			}

			if (it_type->second.AsString() == "Map") {
				json::Dict r;
				r["request_id"] = it_id->second.AsInt();

				std::ostringstream s;
				rh.RenderMap().Render(s);
				r["map"] = s.str();
				gl.push_back(r);

				continue;
			}

			auto it_name = map.find("name");
			if (it_name == map.end()) {
				throw;
			}

			if (it_type->second.AsString() == "Stop") {
				const std::set<std::string_view> buses = catalogue.GetStopStationInfo(it_name->second.AsString());
				json::Dict r;
				json::Array g;
				for (auto& bus : buses) {
					g.push_back(json::Node(std::string(bus.data())));
				}

				if (catalogue.GetStopStation(it_name->second.AsString()) == nullptr) {
					r["request_id"] = it_id->second.AsInt();
					r["error_message"] = "not found"s;
				}
				else
				{
					json::Array g;
					for (auto bus : buses) {
						g.push_back(json::Node(std::string(bus.data())));
					}
					r["buses"] = g;
					r["request_id"] = it_id->second.AsInt();
				}

				gl.push_back(r);
			}
			else if (it_type->second.AsString() == "Bus") {
				auto b = catalogue.GetBusInfo(it_name->second.AsString());
				json::Dict r;
				if (!b.has_value()) {
					r["request_id"] = it_id->second.AsInt();
					r["error_message"] = "not found"s;
				}
				else {
					r["curvature"] = b.value().curvature;
					r["request_id"] = it_id->second.AsInt();
					r["route_length"] = b.value().route_length;
					r["stop_count"] = static_cast<int>(b.value().stops_count);
					r["unique_stop_count"] = static_cast<int>(b.value().unique_stops_count);
				}
				gl.push_back(r);
			}

		}
		const json::Document result(gl);
		return result;
	}
}