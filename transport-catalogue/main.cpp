#include <iostream>
#include <fstream>
#include "json_reader.h"
#include "transport_catalogue.h"
// #include "duration/log_duration.h"

using namespace std;
using namespace t_c;

int main() {
    {
        TransportCatalogue catalogue;
        json_reader::LoadJSON(cin, cout, catalogue);
    }

    // {
    //     ifstream input_file("../examples/1_example/inp.json");
    //     ofstream output_file("./output/1out.json");
    //     TransportCatalogue catalogue;
    //     json_reader::LoadJSON(input_file, output_file, catalogue);
    // }
    // {
    //     ifstream input_file("../examples/2_example/inp.json");
    //     ofstream output_file("./output/2out.json");
    //     TransportCatalogue catalogue;
    //     json_reader::LoadJSON(input_file, output_file, catalogue);
    // }
    // {
    //     ifstream input_file("../examples/3_example/inp.json");
    //     ofstream output_file("./output/3out.json");
    //     TransportCatalogue catalogue;
    //     json_reader::LoadJSON(input_file, output_file, catalogue);
    // }
    // {
    //     LOG_DURATION("4_test");
    //     ifstream input_file("../examples/4_example/inp.json");
    //     ofstream output_file("./output/4out.json");
    //     TransportCatalogue catalogue;
    //     json_reader::LoadJSON(input_file, output_file, catalogue);
    // }
}