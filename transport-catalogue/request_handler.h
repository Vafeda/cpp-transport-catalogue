#pragma once
#include "transport_catalogue.h"
#include "map_renderer.h"

class RequestHandler {
public:
    RequestHandler(const transport_catalogue::TransportCatalogue& db, const map_renderer::RenderSettings& rs)
        : db_(db)
        , rs_(rs)
    {
    }

    svg::Document RenderMap() const;

private:

    const transport_catalogue::TransportCatalogue& db_;
    const map_renderer::RenderSettings& rs_;
};