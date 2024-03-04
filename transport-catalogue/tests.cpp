#include "tests.h"

using namespace std;
using namespace i_r;
using namespace o_r;
using t_c::TransportCatalogue;

bool CompareAnswers(const std::string& prog_res, const std::string& exp_res) {
    if (prog_res.size() != exp_res.size()) {
        cerr << "Size of answers fault. Program size is " << prog_res.size() << " vs expected " << exp_res.size() << '\n';
        return false;
    }

    for (size_t i = 0; i < prog_res.size(); ++i) {
        if (prog_res[i] != exp_res[i]) {
            cerr << "Answers differ at position " << i << ": '" << prog_res[i] << "' vs '" << exp_res[i] << "'\n";
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

    InputReader inp;
    inp.ReadData(input, catalogue);

    // get and process requests
    ostringstream program_result;
    ProcessRequest(input, program_result, catalogue);

    // read correct result from file (res_filename)
    string correct_output_str, line;
    while (getline(correct_output, line)) {
        correct_output_str += line + "\n";
    }

    // read result of program
    string program_result_str = program_result.str();

    // write both results in files for next comparation
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

    // compare program answer and expected output (res_filename)
    return CompareAnswers(program_result_str, correct_output_str);
}

void TestCatalogue::tsC_case0() {
    if (TestsTemplate("./test_templates/tsC_case0_input.txt", "./test_templates/tsC_case0_output.txt")) {
        cerr << "test 0, case 0 PASSED" << endl;
    } else {
        cerr << "test 0, case 0 FAILED" << endl;
    }
}

void TestCatalogue::tsC_case1() {
    if (TestsTemplate("./test_templates/tsC_case1_input.txt", "./test_templates/tsC_case1_output1.txt")) {
        cerr << "test C, case 1 PASSED" << endl;
    } else {
        cerr << "test C, case 1 FAILED" << endl;
    }
}

void TestCatalogue::tsC_case2() {
    if (TestsTemplate("./test_templates/tsC_case2_input.txt", "./test_templates/tsC_case2_output.txt")) {
        cerr << "test C, case 2 PASSED" << endl;
    } else {
        cerr << "test C, case 2 FAILED" << endl;
    }
}