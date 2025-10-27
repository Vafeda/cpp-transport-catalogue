#pragma once

#include "transport_catalogue.h"
#include "graph.h"
#include "router.h"

#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>

namespace graph {

    struct RouteSetting {
        int bus_wait_time = 0;
        int bus_velocity = 0;
    };

    struct BusEdgeData {
        std::string bus_name;
        int span_count = 0;
        std::string from_stop;
        std::string to_stop;
    };

    template <typename Weight>
    class TransportGraph {
    private:
        DirectedWeightedGraph<Weight> graph_;
        int bus_velocity_;
        int bus_wait_time_;

        std::unordered_map<std::string, size_t> stop_to_wait_vertex_;
        std::unordered_map<std::string, size_t> stop_to_bus_vertex_;
        std::unordered_map<size_t, std::string> vertex_to_stop_;

        std::unordered_map<size_t, BusEdgeData> bus_edges_;

    public:
        TransportGraph(const transport_catalogue::TransportCatalogue& catalogue, const RouteSetting& rs)
            : bus_velocity_(rs.bus_velocity), bus_wait_time_(rs.bus_wait_time) {
            BuildGraph(catalogue);
        }

        const DirectedWeightedGraph<Weight>& GetGraph() const { return graph_; }
        const std::unordered_map<size_t, BusEdgeData>& GetBusEdges() const { return bus_edges_; }

        size_t GetWaitVertex(const std::string& stop_name) const {
            return stop_to_wait_vertex_.at(stop_name);
        }

        const std::string& GetStopName(size_t vertex_id) const {
            return vertex_to_stop_.at(vertex_id);
        }

    private:
        void BuildGraph(const transport_catalogue::TransportCatalogue& catalogue) {
            const auto& all_stops = catalogue.GetAllStops();
            graph_ = DirectedWeightedGraph<Weight>(all_stops.size() * 2);

            size_t vertex_id = 0;
            for (const auto& stop : all_stops) {
                stop_to_wait_vertex_[stop.name] = vertex_id;
                stop_to_bus_vertex_[stop.name] = vertex_id + 1;

                vertex_to_stop_[vertex_id] = stop.name;
                vertex_to_stop_[vertex_id + 1] = stop.name;

                graph_.AddEdge({
                    vertex_id,
                    vertex_id + 1,
                    static_cast<Weight>(bus_wait_time_)
                    });

                vertex_id += 2;
            }

            for (const auto& bus : catalogue.GetAllRoute()) {
                AddBusEdges(catalogue, bus);
            }
        }

        void AddBusEdges(const transport_catalogue::TransportCatalogue& catalogue,
            const transport_catalogue::Bus& bus) {
            const auto& stops = bus.route;

            if (bus.is_roundtrip) {
                for (size_t i = 0; i < stops.size(); ++i) {
                    double total_distance = 0;
                    for (size_t j = i + 1; j < stops.size(); ++j) {
                        auto dist = catalogue.GetDistanceBetweenStopsStations(stops[j - 1], stops[j]);
                        if (!dist) {
                            dist = catalogue.GetDistanceBetweenStopsStations(stops[j], stops[j - 1]);
                        }
                        if (dist) {
                            total_distance += *dist;
                            AddBusEdge(bus.name, stops[i]->name, stops[j]->name,
                                j - i, total_distance);
                        }
                    }
                }
            }
            else {
                for (size_t i = 0; i < stops.size(); ++i) {
                    double total_distance = 0;
                    for (size_t j = i + 1; j < stops.size(); ++j) {
                        auto dist = catalogue.GetDistanceBetweenStopsStations(stops[j - 1], stops[j]);
                        if (!dist) {
                            dist = catalogue.GetDistanceBetweenStopsStations(stops[j], stops[j - 1]);
                        }
                        if (dist) {
                            total_distance += *dist;
                            AddBusEdge(bus.name, stops[i]->name, stops[j]->name,
                                j - i, total_distance);
                        }
                    }
                }

                for (size_t i = stops.size() - 1; i > 0; --i) {
                    double total_distance = 0;
                    for (size_t j = i - 1; j != static_cast<size_t>(-1); --j) {
                        auto dist = catalogue.GetDistanceBetweenStopsStations(stops[j + 1], stops[j]);
                        if (!dist) {
                            dist = catalogue.GetDistanceBetweenStopsStations(stops[j], stops[j + 1]);
                        }
                        if (dist) {
                            total_distance += *dist;
                            AddBusEdge(bus.name, stops[i]->name, stops[j]->name,
                                i - j, total_distance);
                        }
                    }
                }
            }
        }

        void AddBusEdge(const std::string& bus_name, const std::string& from_stop,
            const std::string& to_stop, int span_count, double total_distance) {
            double time_minutes = (total_distance / 1000.0) / bus_velocity_ * 60.0;

            size_t from_vertex = stop_to_bus_vertex_.at(from_stop);
            size_t to_vertex = stop_to_wait_vertex_.at(to_stop);

            size_t edge_id = graph_.AddEdge({
                from_vertex,
                to_vertex,
                static_cast<Weight>(time_minutes)
                });

            bus_edges_[edge_id] = {
                bus_name,
                span_count,
                from_stop,
                to_stop
            };
        }
    };

    template <typename Weight>
    class TransportRouter {
    private:
        TransportGraph<Weight> graph_;
        Router<Weight> router_;

    public:
        TransportRouter(const transport_catalogue::TransportCatalogue& catalogue,
            const RouteSetting& rs)
            : graph_(catalogue, rs)
            , router_(graph_.GetGraph()) { 
        }

        struct RouteItem {
            enum class Type { WAIT, BUS };
            Type type;
            std::string stop_name;
            std::string bus_name;
            Weight time;
            int span_count = 0;
        };

        struct RouteResult {
            Weight total_time = 0;
            std::vector<RouteItem> items;
        };

        std::optional<RouteResult> FindRoute(const std::string& from, const std::string& to) const {
            try {
                size_t from_vertex = graph_.GetWaitVertex(from);
                size_t to_vertex = graph_.GetWaitVertex(to);

                auto route_info = router_.BuildRoute(from_vertex, to_vertex);
                if (!route_info) {
                    return std::nullopt;
                }

                return BuildRouteResult(*route_info);
            }
            catch (const std::exception&) {
                return std::nullopt;
            }
        }

    private:
        RouteResult BuildRouteResult(const typename Router<Weight>::RouteInfo& route_info) const {
            RouteResult result;
            result.total_time = route_info.weight;

            const auto& bus_edges = graph_.GetBusEdges();
            const auto& graph = graph_.GetGraph();

            for (size_t edge_id : route_info.edges) {
                const auto& edge = graph.GetEdge(edge_id);

                auto bus_edge_it = bus_edges.find(edge_id);
                if (bus_edge_it != bus_edges.end()) {
                    result.items.push_back({
                        RouteItem::Type::BUS,
                        "",
                        bus_edge_it->second.bus_name,
                        edge.weight,
                        bus_edge_it->second.span_count
                        });
                }
                else {
                    const std::string& stop_name = graph_.GetStopName(edge.from);
                    result.items.push_back({
                        RouteItem::Type::WAIT,
                        stop_name,
                        "", 
                        edge.weight,
                        0
                        });
                }
            }

            return result;
        }
    };
}