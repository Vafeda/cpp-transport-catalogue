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

		const json::Array array = base_requests->second.AsArray();
		ProcessStopRequests(array, catalogue);
		ProcessBusRequests(array, catalogue);

		return catalogue;
	}
	
	void JsonReader::ProcessStopRequests(const json::Array& array, transport_catalogue::TransportCatalogue& tc) const {
		for (const json::Node& a : array) {
			json::Dict map;

			map = a.AsDict();

			auto it_type = map.find("type");
			if (it_type == map.end()) {
				throw std::logic_error(error_messeg_base_requests_type);
			}

			if (it_type->second.AsString() == "Stop") {
				ApplyStopInfo(map, tc);
			}
		}
	}

	void JsonReader::ProcessBusRequests(const json::Array& array, transport_catalogue::TransportCatalogue& tc) const {
		for (const json::Node& a : array) {
			json::Dict map;

			map = a.AsDict();

			auto it_type = map.find("type");
			if (it_type == map.end()) {
				throw std::logic_error(error_messeg_base_requests_type);
			}

			if (it_type->second.AsString() == "Bus") {
				ApplyBusInfo(map, tc);
			}
		}
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

			tc.AddStopStation(it_name->second.AsString(), { it_lat->second.AsDouble(), it_lon->second.AsDouble() });
		}

		// Apply road distance between StopStations (for success need to know "name", std::map<"name", int>)
		{
			auto it_road_distances = dict.find("road_distances");
			if (it_road_distances == dict.end()) {
				throw std::logic_error(error_messeg_base_requests_stop + "\"road_distances\"");
			}

			const json::Dict road_distances = it_road_distances->second.AsDict();
			for (const auto& [key, value] : road_distances) {
				tc.SetDistanceBetweenStopsStations(it_name->second.AsString(), key, value.AsInt());
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

	//-------------------------------------------------------------------
	//----------------Function Apply Rendering Settings------------------
	//-------------------------------------------------------------------
	inline std::string error_messeg_render_setting = "JsonReader(ApplyRenderSetting): The \"render_settings\" dictionary missing the key "s;
	
	map_renderer::RenderSettings JsonReader::ApplyRenderSettings() const {
		auto render_settings = json_.find(render_key);
		if (render_settings == json_.end()) {
			throw std::logic_error("JsonReader(ApplyRenderSetting): The dictionary is missing a key\"" + render_key + "\"");
		}

		const json::Dict dict = render_settings->second.AsDict();
			
		map_renderer::RenderSettings rs;
		ApplyWidthHeightPadding(dict, rs);
		ApplyLineWidthStopRadius(dict, rs);
		ApplyBusLabel(dict, rs);
		ApplyStopLabel(dict, rs);
		ApplyUnderlayer(dict, rs);
		ApplyColorPalette(dict, rs);
		return rs;
	}

	void JsonReader::ApplyWidthHeightPadding(const json::Dict& dict, map_renderer::RenderSettings& rs) const {
		// Apply width
		auto it_width = dict.find("width"s);
		if (it_width == dict.end()) {
			throw std::logic_error(error_messeg_render_setting + "\"width\""s);
		}
		double width = it_width->second.AsDouble();
		if (!(width >= 0.0 && width <= 100000.0)) {
			throw std::invalid_argument("Width must be in range [0, 100000]"s);
		}
		rs.width_ = width;

		// Apply height 
		auto it_height = dict.find("height"s);
		if (it_height == dict.end()) {
			throw std::logic_error(error_messeg_render_setting + "\"height\""s);
		}
		double height = it_height->second.AsDouble();
		if (!(height >= 0.0 && height <= 100000.0)) {
			throw std::invalid_argument("Height must be in range [0, 100000]"s);
		}
		rs.height_ = height;

		// Apply padding
		auto it_padding = dict.find("padding"s);
		if (it_padding == dict.end()) {
			throw std::logic_error(error_messeg_render_setting + "\"padding\"");
		}
		double upper_limit = std::min(width, height) / 2;
		double padding = it_padding->second.AsDouble();
		if (!(padding >= 0.0 && padding <= upper_limit)) {
			throw std::invalid_argument("Padding must be in range [0, " + std::to_string(upper_limit) + "]");
		}
		rs.padding_ = padding;
	}

	void JsonReader::ApplyLineWidthStopRadius(const json::Dict& dict, map_renderer::RenderSettings& rs) const {
		// Apply line_width
		auto it_line_width = dict.find("line_width"s);
		if (it_line_width == dict.end()) {
			throw std::logic_error(error_messeg_render_setting + "\"line_width\""s);
		}
		double line_width = it_line_width->second.AsDouble();
		if (!(line_width >= 0.0 && line_width <= 100000.0)) {
			throw std::invalid_argument("Line_width must be in range [0, 100000]"s);
		}
		rs.line_width_ = line_width;

		// Apply stop_radius
		auto it_stop_radius = dict.find("stop_radius"s);
		if (it_stop_radius == dict.end()) {
			throw std::logic_error(error_messeg_render_setting + "\"stop_radius\""s);
		}
		double stop_radius = it_stop_radius->second.AsDouble();
		if (!(stop_radius >= 0.0 && stop_radius <= 100000.0)) {
			throw std::invalid_argument("Stop_radius must be in range [0, 100000]"s);
		}
		rs.stop_radius_ = stop_radius;
	}

	void JsonReader::ApplyBusLabel(const json::Dict& dict, map_renderer::RenderSettings& rs) const {
		// Apply bus_label_font_size
		auto it_bus_label_font_size = dict.find("bus_label_font_size"s);
		if (it_bus_label_font_size == dict.end()) {
			throw std::logic_error(error_messeg_render_setting + "\"bus_label_font_size\""s);
		}
		int bus_label_font_size = it_bus_label_font_size->second.AsInt();
		if (!(bus_label_font_size >= 0 && bus_label_font_size <= 100000)) {
			throw std::invalid_argument("Bus_label_font_size must be in range [0, 100000]"s);
		}
		rs.bus_label_font_size_ = bus_label_font_size;

		// Apply bus_label_offset
		auto it_bus_label_offset = dict.find("bus_label_offset"s);
		if (it_bus_label_offset == dict.end()) {
			throw std::logic_error(error_messeg_render_setting + "\"bus_label_offset\""s);
		}
			
		json::Array array = it_bus_label_offset->second.AsArray();
		std::vector<double> bus_label_offset;
		for (auto& node : array) {
			double n = node.AsDouble();
			if (!(n >= -100000.0 && n <= 100000.0)) {
				throw std::invalid_argument("Bus_label_offset element must be in range [-100000, 100000]"s);
			}
			bus_label_offset.push_back(n);
		}

		if (!(bus_label_offset.size() == 2)) {
			throw std::invalid_argument("Bus_label_offset must contain exactly 2 elements (dx and dy)"s);
		}

		rs.bus_label_offset_ = bus_label_offset;
	}

	void JsonReader::ApplyStopLabel(const json::Dict& dict, map_renderer::RenderSettings& rs) const {
		// Apply stop_label_font_size
		auto it_stop_label_font_size = dict.find("stop_label_font_size"s);
		if (it_stop_label_font_size == dict.end()) {
			throw std::logic_error(error_messeg_render_setting + "\"stop_label_font_size\""s);
		}
		int stop_label_font_size = it_stop_label_font_size->second.AsInt();
		if (!(stop_label_font_size >= 0 && stop_label_font_size <= 100000)) {
			throw std::invalid_argument("Stop_label_font_size must be in range [0, 100000]"s);
		}
		rs.stop_label_font_size_ = stop_label_font_size;

		auto it_stop_label_offset = dict.find("stop_label_offset"s);
		if (it_stop_label_offset == dict.end()) {
			throw std::logic_error(error_messeg_render_setting + "\"stop_label_offset\""s);
		}

		// Apply stop_label_offset
		json::Array array = it_stop_label_offset->second.AsArray();
		std::vector<double> stop_label_offset;
		for (auto& node : array) {
			double n = node.AsDouble();
			if (!(n >= -100000.0 && n <= 100000.0)) {
				throw std::invalid_argument("Stop_label_offset element must be in range [-100000, 100000]"s);
			}
			stop_label_offset.push_back(n);
		}

		if (!(stop_label_offset.size() == 2)) {
			throw std::invalid_argument("Stop_label_offset must contain exactly 2 elements (dx and dy)"s);
		}

		rs.stop_label_offset_ = stop_label_offset;
	}

	void JsonReader::ApplyUnderlayer(const json::Dict& dict, map_renderer::RenderSettings& rs) const {
		// Apply underlayer_color
		auto it_underlayer_color = dict.find("underlayer_color"s);
		if (it_underlayer_color == dict.end()) {
			throw std::logic_error(error_messeg_render_setting + "\"underlayer_color\""s);
		}
		rs.underlayer_color_ = ParseColorFromJson(it_underlayer_color->second);

		// Apply underlayer_width
		auto it_underlayer_width = dict.find("underlayer_width"s);
		if (it_underlayer_width == dict.end()) {
			throw std::logic_error(error_messeg_render_setting + "\"underlayer_width\""s);
		}
		double underlayer_width = it_underlayer_width->second.AsDouble();
		if (!(underlayer_width >= 0.0 && underlayer_width <= 100000.0)) {
			throw std::invalid_argument("Underlayer_width must be in range [0, 100000]"s);
		}
		rs.underlayer_width_ = underlayer_width;
	}

	void JsonReader::ApplyColorPalette(const json::Dict& dict, map_renderer::RenderSettings& rs) const {
		// Apply color_palette
		auto it_color_palette = dict.find("color_palette"s);
		if (it_color_palette == dict.end()) {
			throw std::logic_error(error_messeg_render_setting + "\"color_palette\""s);
		}

		json::Array array = it_color_palette->second.AsArray();
		std::vector<svg::Color> result;
		for (const json::Node& node : array) {
			result.push_back(ParseColorFromJson(node));
		}

		if (result.empty()) {
			throw std::invalid_argument("Color_palette cannot be empty"s);
		}

		rs.color_palette_ = result;
	}

	const svg::Color JsonReader::ParseColorFromJson(const json::Node& clr) const {
		svg::Color result;

		if (clr.IsString()) {
			result = clr.AsString();
		}
		else if (clr.IsArray()) {
			json::Array color = clr.AsArray();
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

		if (std::holds_alternative<std::monostate>(result)) {
			throw std::invalid_argument("Underlayer color is not specified"s);
		}

		return result;
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

		json::Array stat_info;
		for (const json::Node& a : array) {
			json::Dict map;

			map = a.AsDict();

			auto it_id = map.find("id");
			if (it_id == map.end()) {
				throw std::logic_error("Missing \"id\" field in \"stat_request\"");
			}

			auto it_type = map.find("type");
			if (it_type == map.end()) {
				throw std::logic_error("Missing \"type\" field in \"stat_request\"");
			}

			if (it_type->second.AsString() == "Map") {
				stat_info.push_back(StatMapInfo(it_id->second.AsInt(), rh));
				continue;
			}

			auto it_name = map.find("name");
			if (it_name == map.end()) {
				throw std::logic_error("Missing \"name\" field in \"stat_request\"");
			}

			if (it_type->second.AsString() == "Stop") {
				stat_info.push_back(StatStopInfo(it_id->second.AsInt(), it_name->second.AsString(), catalogue));
			}
			else if (it_type->second.AsString() == "Bus") {
				stat_info.push_back(StatBusInfo(it_id->second.AsInt(), it_name->second.AsString(), catalogue));
			}
		}

		return json::Document(stat_info);
	}

	const json::Dict JsonReader::StatStopInfo(int id, std::string name, const transport_catalogue::TransportCatalogue& catalogue) const {
		const std::set<std::string_view> buses = catalogue.GetStopStationInfo(name);

		json::Dict result;
		
		if (catalogue.GetStopStation(name) == nullptr) {
			result = json::Builder{}.StartDict()
				.Key("request_id"s).Value(id)
				.Key("error_message"s).Value("not found"s)
				.EndDict()
				.Build().AsDict();
		}
		else
		{
			json::Array bus_array;
			for (auto bus : buses) {
				bus_array.push_back(json::Node(std::string(bus.data())));
			}

			result = json::Builder{}.StartDict()
				.Key("buses"s).Value(bus_array)
				.Key("request_id"s).Value(id)
				.EndDict()
				.Build().AsDict();
		}

		return result;
	}

	const json::Dict JsonReader::StatBusInfo(int id, std::string name, const transport_catalogue::TransportCatalogue& catalogue) const {
		std::optional<transport_catalogue::RouteInfo> bus_info = catalogue.GetBusInfo(name);

		json::Dict result;

		if (!bus_info.has_value()) {
			result = json::Builder{}.StartDict()
				.Key("request_id"s).Value(id)
				.Key("error_message"s).Value("not found"s)
				.EndDict()
				.Build().AsDict();
		}
		else {
			const auto& bus = bus_info.value();

			result = json::Builder{}.StartDict()
				.Key("curvature"s).Value(bus.curvature)
				.Key("request_id"s).Value(id)
				.Key("route_length"s).Value(bus.route_length)
				.Key("stop_count"s).Value(static_cast<int>(bus.stops_count))
				.Key("unique_stop_count"s).Value(static_cast<int>(bus.unique_stops_count))
				.EndDict()
				.Build().AsDict();
		}

		return result;
	}

	const json::Dict JsonReader::StatMapInfo(int id, const RequestHandler& rh) const {
		json::Dict result;

		std::ostringstream s;
		rh.RenderMap().Render(s);

		result = json::Builder{}.StartDict()
			.Key("request_id"s).Value(id)
			.Key("map"s).Value(s.str())
			.EndDict()
			.Build().AsDict();

		return result;
	}
}