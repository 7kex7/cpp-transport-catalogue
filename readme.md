<b>TRANSPORT CATALOGUE</b>
Транспортный справочник - это приложение на C++, позволяющее 

1) создать карту транспортных сообщений из названий остановок и линий-путей в svg формате;

2) вернуть информацию об остановке/автобусе.

В проекте своя реализация json и svg библиотек.

На вход подаётся json-файл в виде:
```json
{
    "base_requests": [
    {
        "type": "Bus",
        "name": "Name_Of_Route",
        "stops": ["<Existed_stop>", "<Existed_stop2>"],
        "is_roundtrip": <is_route_roundtrip_boolean>
    },
    {
        "type": "Stop",
        "name": "Name_Of_Stop",
        "latitude": value[-90 : +90],
        "longitude": value[-180 : +180],
        "road_distances": {"<Stop_to>": <distance_in_meters>}
    },
    ],
    "render_settings": { "<prop>": <attr>, "<prop2>": <attr> },
    "stat_requests": [
        { "id": 1, "type": "Map" },
        { "id": 2, "type": "Stop", "name": "Name_Of_Stop" },
        { "id": 3, "type": "Bus", "name": "Name_Of_Route" }
    ]
}
```

``` "base_requests" ``` - Массив для заполнения из запросов двух типов: остановки (Stop) и маршрута (Bus). Помимо имён, для остановки заполняются долгота и широта + расстояния до соседних остановок

``` "render_settings" ``` - map-объект для настроек вывода карты:
"width", "height", "padding", "stop_radius", "line_width", "bus_label_font_size", "bus_label_offset", "stop_label_font_size", "stop_label_offset", "underlayer_color", "underlayer_width", "color_palette".

``` "stat_requests" ``` - массив для запросов на вывод трёх типов: Map - отрисовка карты, Stop - информация об остановке, Bus - информация о маршруте.

в файле ./transport-catalogue/input/input.json приведён пример запросов
Вывод и ввод осуществляется через стандартные потоки чтения/записи.
