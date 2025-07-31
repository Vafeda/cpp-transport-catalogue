#pragma once
#include "geo.h"
#include <vector>
#include <string>

namespace transport_catalogue {
	struct StopStation {
		std::string name;
		geo::Coordinates coordinates;
	};

	bool operator<(const StopStation& lhs, const StopStation& rhs);

	struct Bus {
		std::string name;
		std::vector<const StopStation*> route;
		bool is_roundtrip;
	};

	struct RouteInfo {
		size_t stops_count;
		size_t unique_stops_count;
		int route_length;
		double curvature;
		bool route_exists;
	};

}