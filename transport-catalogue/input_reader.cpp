#include "input_reader.h"

using namespace std::literals;
 
namespace i_r {

void InputReader::ReadData(std::istream& input, t_c::TransportCatalogue& catalogue) {
    int base_request_count;
    input >> base_request_count >> std::ws;

    {
        for (int i = 0; i < base_request_count; ++i) {
            std::string line;
            getline(input, line);
            this->ParseLine(line);
        }
        this->ApplyCommands(catalogue);
    }
}

/**
 * Удаляет пробелы в начале и конце строки
 */
std::string_view Trim(std::string_view string) {
    const auto start = string.find_first_not_of(' ');
    if (start == string.npos) {
        return {};
    }
    return string.substr(start, string.find_last_not_of(' ') + 1 - start);
}

/**
 * Разбивает строку string на n строк, с помощью указанного символа-разделителя delim
 */
std::vector<std::string_view> Split(std::string_view string, char delim) {
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
std::vector<std::string_view> ParseRoute(std::string_view route) {
    if (route.find('>') != route.npos) {
        return Split(route, '>');
    }
 
    auto stops = Split(route, '-');
    std::vector<std::string_view> results(stops.begin(), stops.end());
    results.insert(results.end(), std::next(stops.rbegin()), stops.rend());

    return results;
}

CommandDescription ParseCommandDescription(std::string_view line) {
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

    return {std::string(line.substr(0, space_pos)),
            std::string(line.substr(not_space, colon_pos - not_space)),
            std::string(line.substr(colon_pos + 1))};
}

void InputReader::ParseLine(std::string_view line) {
    auto command_description = ParseCommandDescription(line);
    if (command_description) {
        commands_.push_back(std::move(command_description));
    }
}

/**
 * Парсит строку вида "10.123,  -30.1837" и возвращает пару координат (широта, долгота)
*/
Coordinates ParseCoordinates(std::string_view& str) {
    static const double nan = std::nan("");

    auto not_space = str.find_first_not_of(' ');
    auto comma = str.find(',');

    if (comma == str.npos) {
        return {nan, nan};
    }

    auto not_space2 = str.find_first_not_of(' ', comma + 1);
    auto comma2 = str.find(' ', not_space2);

    const double lat = std::stod(std::string(str.substr(not_space, comma - not_space)));
    const double lng = std::stod(std::string(str.substr(not_space2, comma2 - not_space2)));

    return {lat, lng};
}

double GetMeters(std::string_view distance) {
    std::string_view meters = distance.substr(0, distance.find(' '));
    return std::stod(std::string(meters.substr(0, meters.size() - 1)));
}

std::string_view GetStopName(std::string_view distance) {
    size_t first_space = distance.find(' ');
    size_t sec_space = distance.find(' ', first_space + 1);
    std::string_view stopnm = distance.substr(sec_space + 1, distance.size() - sec_space);
    return stopnm;
}

void ApplyDistances(const CommandDescription& cmd
                , t_c::TransportCatalogue& catalogue) {
    t_c::Stop* from_stop = &catalogue.FindStop(cmd.id);
    std::string_view description = cmd.description;
    size_t first_comma = description.find(',');
    size_t current_comma = description.find(',', first_comma + 1);

    while (current_comma != description.npos) {
        size_t first_after_comma = current_comma + 1;
        size_t next_comma = description.find(',', first_after_comma);

        std::string_view distance;
        if (next_comma == description.npos) {
            distance = description.substr(first_after_comma, description.size() - first_after_comma);
        } else {
            distance = description.substr(first_after_comma, next_comma - first_after_comma);
        }
        // get meters
        distance = Trim(distance);

        double meters = GetMeters(distance);
        // get stop
        t_c::Stop* to_stop = &catalogue.FindStop(GetStopName(distance));

        catalogue.AddDistance(from_stop, to_stop, meters);
        current_comma = next_comma;
    }
}

void InputReader::ApplyCommands([[maybe_unused]] t_c::TransportCatalogue& catalogue) const {
    std::vector<CommandDescription> bus_cmds;

    // apply stops and get buses in container
    for (const auto& cmd : commands_) {
        if (cmd.command == "Bus"s) {
            bus_cmds.push_back(cmd);
        } else {
            std::string_view description = cmd.description;
            Coordinates coords = ParseCoordinates(description);
            catalogue.AddStop(t_c::Stop(cmd.id, coords));
        }
    }

    // add buses
    for (const auto& bus_cmd : bus_cmds) {
        std::vector<std::string_view> route = ParseRoute(bus_cmd.description);
        std::vector<t_c::Stop*> stops(route.size());
        for (size_t i = 0; i < route.size(); ++i) {
            stops[i] = &catalogue.FindStop(route[i]);
        }
        catalogue.AddBus(t_c::Bus(bus_cmd.id, stops));
    }

    // add distances
    for (const auto& cmd : commands_) {
        if (cmd.command != "Stop"s) {
            continue;
        }
        ApplyDistances(cmd, catalogue);
    }
}

} // i_r