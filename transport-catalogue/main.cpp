#include <iostream>
#include <fstream>
#include "json_reader.h"
#include "transport_catalogue.h"

using namespace std;
using namespace t_c;

int main() {
    // ifstream input_file("./input/input.json");
    // ofstream output_file("./output/output.json");
    // TransportCatalogue catalogue;
    // json_reader::LoadJSON(input_file, output_file, catalogue);
    // input_file.close();
    // output_file.close();

    TransportCatalogue catalogue;
    json_reader::LoadJSON(cin, cout, catalogue);
}