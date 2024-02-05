#include "tests.h"

using namespace std;
using namespace i_r;
using namespace o_r;
using t_c::TransportCatalogue;

bool CompareStrings(const std::string& str1, const std::string& str2) {
    // if (str1.size() != str2.size()) {
    //     cerr << "size of strings fault: " << str1.size() << " and " << str2.size() << endl;
    //     return false;
    // }

    for (size_t i = 0; i < str1.size(); ++i) {
        if (str1[i] != str2[i]) {
            cerr << "Strings differ at position " << i << ": '" << str1[i] << "' vs '" << str2[i] << "'\n";
            return false;
        }
    }

    return true;
}

bool TestsTemplate(const string& in_filename, const string& res_filename) {
    std::ifstream input(in_filename);
    std::ifstream correct_output(res_filename);
    if (!input.is_open() && correct_output.is_open()) {
        cerr << "failed to open files" << endl;
        return false;
    } else if (!input.is_open() && correct_output.is_open()) {
        correct_output.close();
        cerr << "failed to open file " << in_filename << endl;
        return false;
    } else if (!correct_output.is_open() && input.is_open()) {
        input.close();
        cerr << "failed to open file " << res_filename << endl;
        return false;
    }

    TransportCatalogue catalogue;
    int base_request_count;
    input >> base_request_count >> ws;
    {
        InputReader reader;
        for (int i = 0; i < base_request_count; ++i) {
            string line;
            getline(input, line);
            reader.ParseLine(line);
        }
        reader.ApplyCommands(catalogue);
    }

    // do calculations from input file data
    ostringstream program_result;
    int stat_request_count;
    input >> stat_request_count >> ws;
    for (int i = 0; i < stat_request_count; ++i) {
        string line;
        getline(input, line);
        ParseAndPrintStat(catalogue, line, program_result);
    }
    input.close();

    // get correct result from file
    string correct_output_str, line;
    while (getline(correct_output, line)) {
        correct_output_str += line + "\n";
    }

    string program_result_str = program_result.str();

    {
        ofstream file_result_of_program("./results/program_result.txt");
        file_result_of_program << program_result_str;
        file_result_of_program.close();
    }
    {
        ofstream file_result_buffer("./results/buffer_result.txt");
        file_result_buffer << correct_output_str;
        file_result_buffer.close();
    }

    correct_output.close();

    return CompareStrings(program_result_str, correct_output_str);
}

void TestCatalogue::tsZERO_caseZERO() {
    if (TestsTemplate("./test_templates/tsZERO_caseZERO_input.txt", "./test_templates/tsZERO_caseZERO_output.txt")) {
        cerr << "test 0, case 0 PASSED" << endl;
    } else {
        cerr << "test 0, case 0 FAILED" << endl;
    }
}

void TestCatalogue::tsA_case1() {
    if (TestsTemplate("./test_templates/tsA_case1_input.txt", "./test_templates/tsA_case1_output.txt")) {
        cerr << "test A, case 1 PASSED" << endl;
    } else {
        cerr << "test A, case 1 FAILED" << endl;
    }
}

void TestCatalogue::tsA_case2() {
    if (TestsTemplate("./test_templates/tsA_case2_input.txt", "./test_templates/tsA_case2_output.txt")) {
        cerr << "test A, case 2 PASSED" << endl;
    } else {
        cerr << "test A, case 2 FAILED" << endl;
    }
}
