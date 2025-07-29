#include "map_renderer.h"

namespace map_renderer {
	
    void RenderSettings::SetWidth(double width) {
        if (!(width >= 0.0 && width <= 100000.0)) {
            throw std::invalid_argument("Width must be in range [0, 100000]"s);
        }
        width_ = width;
    }

    void RenderSettings::SetHeight(double height) {
        if (!(height >= 0.0 && height <= 100000.0)) {
            throw std::invalid_argument("Height must be in range [0, 100000]"s);
        }
        height_ = height;
    }

    void RenderSettings::SetPadding(double padding) {
        double upper_limit = std::min(width_, height_) / 2;
        if (!(padding >= 0.0 && padding <= upper_limit)) {
            throw std::invalid_argument("Padding must be in range [0, " + std::to_string(upper_limit) + "]");
        }

        padding_ = padding;
    }

    void RenderSettings::SetLineWidth(double line_width) {
        if (!(line_width >= 0.0 && line_width <= 100000.0)) {
            throw std::invalid_argument("Line_width must be in range [0, 100000]"s);
        }
        line_width_ = line_width;
    }

    void RenderSettings::SetStopRadius(double stop_radius) {
        if (!(stop_radius >= 0.0 && stop_radius <= 100000.0)) {
            throw std::invalid_argument("Stop_radius must be in range [0, 100000]"s);
        }
        stop_radius_ = stop_radius;
    }

    void RenderSettings::SetBusLabelFontSize(int bus_label_font_size) {
        if (!(bus_label_font_size >= 0 && bus_label_font_size <= 100000)) {
            throw std::invalid_argument("Bus_label_font_size must be in range [0, 100000]"s);
        }
        bus_label_font_size_ = bus_label_font_size;
    }

    void RenderSettings::SetBusLabelOffset(const std::vector<double>& bus_label_offset) {
        if (!(bus_label_offset.size() == 2)) {
            throw std::invalid_argument("Bus_label_offset must contain exactly 2 elements (dx and dy)"s);
        }

        for (const double& n : bus_label_offset) {
            if (!(n >= -100000.0 && n <= 100000.0)) {
                throw std::invalid_argument("Bus_label_offset element must be in range [-100000, 100000]"s);
            }
            bus_label_offset_.push_back(n);
        }
    }

    void RenderSettings::SetStopLabelFontSize(int stop_label_font_size) {
        if (!(stop_label_font_size >= 0 && stop_label_font_size <= 100000)) {
            throw std::invalid_argument("Stop_label_font_size must be in range [0, 100000]"s);
        }
        stop_label_font_size_ = stop_label_font_size;
    }

    void RenderSettings::SetStopLabelOffset(const std::vector<double>& stop_label_offset) {
        if (!(stop_label_offset.size() == 2)) {
            throw std::invalid_argument("Stop_label_offset must contain exactly 2 elements (dx and dy)"s);
        }

        for (const double& n : stop_label_offset) {
            if (!(n >= -100000.0 && n <= 100000.0)) {
                throw std::invalid_argument("Stop_label_offset element must be in range [-100000, 100000]"s);
            }
            stop_label_offset_.push_back(n);
        }
    }

    void RenderSettings::SetUnderlayerColor(const svg::Color& underlayer_color) {
        if (std::holds_alternative<std::monostate>(underlayer_color)) {
            throw std::invalid_argument("Underlayer color is not specified"s);
        }
        underlayer_color_ = underlayer_color;
    }

    void RenderSettings::SetUnderlayerWidth(double underlayer_width) {
        if (!(underlayer_width >= 0.0 && underlayer_width <= 100000.0)) {
            throw std::invalid_argument("Underlayer_width must be in range [0, 100000]"s);
        }
        underlayer_width_ = underlayer_width;
    }

    void RenderSettings::SetColorPalette(const std::vector<svg::Color>& color_palette) {
        if (color_palette.empty()) {
            throw std::invalid_argument("Color_palette cannot be empty"s);
        }

        for (const svg::Color& n : color_palette) {
            if (std::holds_alternative<std::monostate>(n)) {
                throw std::invalid_argument("Underlayer color is not specified"s);
            }
            color_palette_.push_back(n);
        }
    }

    double RenderSettings::GetWidth() const {
        return width_;
    }

    double RenderSettings::GetHeight() const {
        return height_;
    }

    double RenderSettings::GetPadding() const {
        return padding_;
    }

    double RenderSettings::GetLineWidth() const {
        return line_width_;
    }

    double RenderSettings::GetStopRadius() const {
        return stop_radius_;
    }

    int RenderSettings::GetBusLabelFontSize() const {
        return bus_label_font_size_;
    }

    const std::vector<double>& RenderSettings::GetBusLabelOffset() const {
        return bus_label_offset_;
    }

    int RenderSettings::GetStopLabelFontSize() const {
        return stop_label_font_size_;
    }

    const std::vector<double>& RenderSettings::GetStopLabelOffset() const {
        return stop_label_offset_;
    }

    const svg::Color& RenderSettings::GetUnderlayerColor() const {
        return underlayer_color_;
    }

    double RenderSettings::GetUnderlayerWidth() const {
        return underlayer_width_;
    }

    const std::vector<svg::Color>& RenderSettings::GetColorPalette() const {
        return color_palette_;
    }

}