#include "json_reader.h"
#include "map_renderer.h"
#include "request_handler.h"

int main() {
    json_reader::JsonReader json(std::cin);

    const transport_catalogue::TransportCatalogue tc = json.ApplyBaseRequests();
    const map_renderer::RenderSettings rs = json.ApplyRenderSettings();
    RequestHandler rh(tc, rs);
        
    json::Print(json.StatInfo(tc, rh), std::cout);
}