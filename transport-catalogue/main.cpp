#include <iostream>
#include <string>

#include "input_reader.h"
#include "stat_reader.h"
#include "transport_catalogue.h"

using namespace std;
using namespace i_r;
using namespace o_r;
using t_c::TransportCatalogue;

int main() {
    TransportCatalogue catalogue;

    InputReader inp;
    inp.ReadData(cin, catalogue);

    ProcessRequest(cin, cout, catalogue);
}

// #include <iostream>
// #include "tests.h"

// using namespace std;

// int main() {
//     TestCatalogue tests;
//     tests.tsC_case0();
//     tests.tsC_case1();
//     // tests.tsC_case2();
// }
