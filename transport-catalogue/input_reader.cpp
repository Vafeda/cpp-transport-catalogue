#include "input_reader.h"

#include <algorithm>
#include <cassert>
#include <iterator>

using namespace transport_catalogue;
/**
 * Парсит строку вида "10.123,  -30.1837" и возвращает пару координат (широта, долгота)
 */
detail::Coordinates input_reader::ParseCoordinates(std::string_view str) {
    static const double nan = std::nan("");

    auto not_space = str.find_first_not_of(' ');
    auto comma = str.find(',');

    if (comma == str.npos) {
        return { nan, nan };
    }

    auto not_space2 = str.find_first_not_of(' ', comma + 1);

    double lat = std::stod(std::string(str.substr(not_space, comma - not_space)));
    double lng = std::stod(std::string(str.substr(not_space2)));

    return { lat, lng };
}

/**
 * Удаляет пробелы в начале и конце строки
 */
std::string_view input_reader::Trim(std::string_view string) {
    const auto start = string.find_first_not_of(' ');
    if (start == string.npos) {
        return {};
    }
    return string.substr(start, string.find_last_not_of(' ') + 1 - start);
}

/**
 * Разбивает строку string на n строк, с помощью указанного символа-разделителя delim
 */
std::vector<std::string_view> input_reader::Split(std::string_view string, char delim) {
    std::vector<std::string_view> result;

    size_t pos = 0;
    while ((pos = string.find_first_not_of(' ', pos)) < string.length()) {
        auto delim_pos = string.find(delim, pos);
        if (delim_pos == string.npos) {
            delim_pos = string.size();
        }
        if (auto substr = Trim(string.substr(pos, delim_pos - pos)); !substr.empty()) {
            result.push_back(substr);
        }
        pos = delim_pos + 1;
    }

    return result;
}

/**
 * Парсит маршрут.
 * Для кольцевого маршрута (A>B>C>A) возвращает массив названий остановок [A,B,C,A]
 * Для некольцевого маршрута (A-B-C-D) возвращает массив названий остановок [A,B,C,D,C,B,A]
 */
std::vector<std::string_view> input_reader::ParseRoute(std::string_view route) {
    if (route.find('>') != route.npos) {
        return Split(route, '>');
    }

    auto stops = Split(route, '-');
    std::vector<std::string_view> results(stops.begin(), stops.end());
    results.insert(results.end(), std::next(stops.rbegin()), stops.rend());

    return results;
}

input_reader::CommandDescription input_reader::ParseCommandDescription(std::string_view line) {
    auto colon_pos = line.find(':');
    if (colon_pos == line.npos) {
        return {};
    }

    auto space_pos = line.find(' ');
    if (space_pos >= colon_pos) {
        return {};
    }

    auto not_space = line.find_first_not_of(' ', space_pos);
    if (not_space >= colon_pos) {
        return {};
    }

    return { std::string(line.substr(0, space_pos)),
            std::string(line.substr(not_space, colon_pos - not_space)),
            std::string(line.substr(colon_pos + 1)) };
}

void input_reader::InputReader::ParseLine(std::string_view line) {
    auto command_description = ParseCommandDescription(line);
    if (command_description) {
        commands_.push_back(std::move(command_description));
    }
}

void input_reader::InputReader::ApplyCommands([[maybe_unused]] TransportCatalogue& catalogue) const {
    std::vector<CommandDescription> sorted_commands = commands_;
    std::sort(sorted_commands.begin(), sorted_commands.end(),
        [](const CommandDescription& lhs, const CommandDescription& rhs) {
            return lhs.command > rhs.command;
        });

    for (auto it_sorted_commands = sorted_commands.begin(); it_sorted_commands < sorted_commands.end(); ++it_sorted_commands) {
        if (it_sorted_commands->command == "Stop") {
            ParseCoordinatesAndLenght(catalogue, it_sorted_commands->id, Split(it_sorted_commands->description, ','));
        }
        else if (it_sorted_commands->command == "Bus") {
            catalogue.SetBus(it_sorted_commands->id, ParseRoute(it_sorted_commands->description));
        }
    }
}

void input_reader::ParseCoordinatesAndLenght(TransportCatalogue& catalogue, const std::string& id, std::vector<std::string_view> parse_string) {
    if (parse_string.size() < 2) {
        catalogue.SetStopStation(id, ParseCoordinates(""));
        return;
    }

    double lat = std::stod(std::string(parse_string[0]));
    double lng = std::stod(std::string(parse_string[1]));
    catalogue.SetStopStation(id, { lat, lng });

    for (auto it = parse_string.begin() + 2; it != parse_string.end(); ++it) {
        size_t pos_between_info = it->find("m to ");
        int distance = std::stoi(std::string(it->substr(0, pos_between_info)));
        std::string stop_station = std::string(it->substr(pos_between_info + 5));
        catalogue.SetDistanceBetweenStopsStations(id, stop_station, distance);
    }
}