#pragma once
#include "transport_catalogue.h"
#include "map_renderer.h"

class RequestHandler {
public:
    RequestHandler(const transport_catalogue::TransportCatalogue& tc, const map_renderer::RenderSettings& rs)
        : tc_(tc)
        , rs_(rs)
    {
    }

    svg::Document RenderMap() const;

private:
    const transport_catalogue::TransportCatalogue& tc_;
    const map_renderer::RenderSettings& rs_;
};