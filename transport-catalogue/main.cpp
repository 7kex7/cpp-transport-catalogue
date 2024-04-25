#include <iostream>
#include <fstream>
#include "json_reader.h"
#include "transport_catalogue.h"

using namespace std;
using namespace t_c;

int main() {
    ifstream input_file("./input/input.json");
    ofstream output_file("./output/out.svg");
    TransportCatalogue catalogue;
    json_reader::LoadJSON(input_file, output_file, catalogue);
    // cout << "hi\n";
    // json_reader::LoadJSON(cin, cout, catalogue);
}