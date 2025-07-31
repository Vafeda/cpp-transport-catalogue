#pragma once
#include "json.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "json_builder.h"
#include <sstream>

// Костыль
#include "request_handler.h"

namespace json_reader {
	using namespace std::literals;

	inline const std::string base_key = "base_requests";
	inline const std::string render_key = "render_settings";
	inline const std::string stat_key = "stat_requests";


	class JsonReader {
	public:
		JsonReader(std::istream& input)
			: input_json_(json::Load(input))
			, json_(input_json_.GetRoot().AsDict())
		{
			if (json_.size() != 3) {
				throw std::logic_error("JsonReader: The dictionary has not 3 elements");
			}

			if (json_.find(base_key) == json_.end()) {
				throw std::logic_error("JsonReader: The dictionary is missing a key\"" + base_key + "\"");
			}

			if (json_.find(render_key) == json_.end()) {
				throw std::logic_error("JsonReader: The dictionary is missing a key\"" + render_key + "\"");
			}

			if (json_.find(stat_key) == json_.end()) {
				throw std::logic_error("JsonReader: The dictionary is missing a key\"" + stat_key + "\"");
			}
		}

		transport_catalogue::TransportCatalogue ApplyBaseRequests() const;
		map_renderer::RenderSettings ApplyRenderSettings() const;
		const json::Document StatInfo(const transport_catalogue::TransportCatalogue& catalogue, const RequestHandler& rh) const;

	private:
		void ProcessStopRequests(const json::Array& array, transport_catalogue::TransportCatalogue& tc) const;
		void ProcessBusRequests(const json::Array& array, transport_catalogue::TransportCatalogue& tc) const;
		void ApplyStopInfo(const json::Dict& dict, transport_catalogue::TransportCatalogue& tc) const;
		void ApplyBusInfo(const json::Dict& dict, transport_catalogue::TransportCatalogue& tc) const;

		void ApplyWidthHeightPadding(const json::Dict& dict, map_renderer::RenderSettings& rs) const;
		void ApplyLineWidthStopRadius(const json::Dict& dict, map_renderer::RenderSettings& rs) const;
		void ApplyBusLabel(const json::Dict& dict, map_renderer::RenderSettings& rs) const;
		void ApplyStopLabel(const json::Dict& dict, map_renderer::RenderSettings& rs) const;
		void ApplyUnderlayer(const json::Dict& dict, map_renderer::RenderSettings& rs) const;
		void ApplyColorPalette(const json::Dict& dict, map_renderer::RenderSettings& rs) const;
		const svg::Color ParseColorFromJson(const json::Node& clr) const;

		const json::Dict StatStopInfo(int id, std::string name, const transport_catalogue::TransportCatalogue& catalogue) const;
		const json::Dict StatBusInfo(int id, std::string name, const transport_catalogue::TransportCatalogue& catalogue) const;
		const json::Dict StatMapInfo(int id, const RequestHandler& rh) const;

	private:
		const json::Document input_json_;
		const json::Dict json_;
	};
}