#pragma once
#include <string>
#include <string_view>
#include <vector>

#include "geo.h"
#include "transport_catalogue.h"

namespace transport_catalogue {
    namespace input_reader {
        struct CommandDescription {
            // Определяет, задана ли команда (поле command непустое)
            explicit operator bool() const {
                return !command.empty();
            }

            bool operator!() const {
                return !operator bool();
            }

            std::string command;      // Название команды
            std::string id;           // id маршрута или остановки
            std::string description;  // Параметры команды
        };

        detail::Coordinates ParseCoordinates(std::string_view str);
        std::string_view Trim(std::string_view string);
        std::vector<std::string_view> Split(std::string_view string, char delim);
        std::vector<std::string_view> ParseRoute(std::string_view route);
        CommandDescription ParseCommandDescription(std::string_view line);
        void ParseCoordinatesAndLenght(TransportCatalogue& catalogue, const std::string& id, std::vector<std::string_view> parse_string);

        class InputReader {
        public:
            /**
             * Парсит строку в структуру CommandDescription и сохраняет результат в commands_
             */
            void ParseLine(std::string_view line);

            /**
             * Наполняет данными транспортный справочник, используя команды из commands_
             */
            void ApplyCommands(TransportCatalogue& catalogue) const;

        private:
            std::vector<CommandDescription> commands_;
        };
    }
}