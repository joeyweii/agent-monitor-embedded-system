#include "test_framework.h"
#include <iostream>
#include <vector>

// Forward declarations of test suites register functions
void register_ring_buffer_tests(std::vector<TestCase>& tests);
void register_protocol_tests(std::vector<TestCase>& tests);
void register_ui_tests(std::vector<TestCase>& tests);
void register_font_tests(std::vector<TestCase>& tests);

int total_assertions = 0;
int failed_assertions = 0;
bool current_test_failed = false;

int main() {
    std::cout << "\033[36m==================================================\033[0m\n";
    std::cout << "\033[36m             Agent Monitor Unit Tests             \033[0m\n";
    std::cout << "\033[36m==================================================\033[0m\n";

    std::vector<TestCase> tests;
    register_ring_buffer_tests(tests);
    register_protocol_tests(tests);
    register_ui_tests(tests);
    register_font_tests(tests);

    int passed_tests = 0;
    int total_tests = 0;

    for (const auto& tc : tests) {
        run_test_case(tc, passed_tests, total_tests);
    }

    std::cout << "\033[36m==================================================\033[0m\n";
    std::cout << "Test Summary:\n";
    std::cout << "  Passed Tests:      " << passed_tests << " / " << total_tests << "\n";
    std::cout << "  Total Assertions:  " << total_assertions << "\n";
    std::cout << "  Failed Assertions: " << failed_assertions << "\n";
    std::cout << "\033[36m==================================================\033[0m\n";

    if (failed_assertions > 0) {
        std::cout << "\033[31mResult: SOME TESTS FAILED!\033[0m\n";
        return 1;
    } else {
        std::cout << "\033[32mResult: ALL TESTS PASSED!\033[0m\n";
        return 0;
    }
}
