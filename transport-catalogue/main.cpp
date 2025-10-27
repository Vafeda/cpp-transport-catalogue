#include "json_reader.h"
#include "map_renderer.h"
#include "request_handler.h"

#include "transport_router.h"

int main() {
    json_reader::JsonReader json(std::cin);

    const transport_catalogue::TransportCatalogue tc = json.ApplyBaseRequests();
    const map_renderer::RenderSettings render_settings = json.ApplyRenderSettings();
    const graph::RouteSetting route_setting = json.ApplyRoutingSetting();
    RequestHandler rh(tc, render_settings);
    
    //graph::TransportGraph<double> tg(tc, route_setting);
    graph::TransportRouter<double> tr(tc, route_setting);

    json::Print(json.StatInfo(tc, rh, tr), std::cout);
}