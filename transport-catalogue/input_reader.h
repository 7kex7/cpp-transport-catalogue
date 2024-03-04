#pragma once
#include <string>
#include <iostream>
#include <string_view>
#include <vector>
#include <cmath>
#include <algorithm>
#include <cassert>
#include <unordered_map>

#include "geo.h"
#include "transport_catalogue.h"


namespace i_r {

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

class InputReader {
public:

    /**
     * Точка входа данных
    */
    void ReadData(std::istream& input, t_c::TransportCatalogue& catalogue);

    /**
     * Парсит строку в структуру CommandDescription и сохраняет результат в commands_
    */
    void ParseLine(std::string_view line);

    /**
     * Наполняет данными транспортный справочник, используя команды из commands_
    */
    void ApplyCommands(t_c::TransportCatalogue& catalogue) const;


private:
    std::vector<CommandDescription> commands_;
};

} // i_r