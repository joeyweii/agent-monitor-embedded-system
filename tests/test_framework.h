#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

#include <iostream>
#include <string>
#include <vector>

extern int total_assertions;
extern int failed_assertions;
extern bool current_test_failed;

#define ASSERT_TRUE(expr) \
    do { \
        total_assertions++; \
        if (!(expr)) { \
            std::cerr << "  [\033[31mFAIL\033[0m] " << __FILE__ << ":" << __LINE__ << ": ASSERT_TRUE(" << #expr << ") failed\n"; \
            current_test_failed = true; \
            failed_assertions++; \
        } \
    } while (0)

#define ASSERT_FALSE(expr) \
    do { \
        total_assertions++; \
        if (expr) { \
            std::cerr << "  [\033[31mFAIL\033[0m] " << __FILE__ << ":" << __LINE__ << ": ASSERT_FALSE(" << #expr << ") failed\n"; \
            current_test_failed = true; \
            failed_assertions++; \
        } \
    } while (0)

#define ASSERT_EQ(actual, expected) \
    do { \
        total_assertions++; \
        auto act = (actual); \
        auto exp = (expected); \
        if (act != exp) { \
            std::cerr << "  [\033[31mFAIL\033[0m] " << __FILE__ << ":" << __LINE__ << ": ASSERT_EQ(" << #actual << ", " << #expected << ") failed\n"; \
            std::cerr << "         Actual:   " << act << "\n"; \
            std::cerr << "         Expected: " << exp << "\n"; \
            current_test_failed = true; \
            failed_assertions++; \
        } \
    } while (0)

#define ASSERT_STR_EQ(actual, expected) \
    do { \
        total_assertions++; \
        std::string act(actual); \
        std::string exp(expected); \
        if (act != exp) { \
            std::cerr << "  [\033[31mFAIL\033[0m] " << __FILE__ << ":" << __LINE__ << ": ASSERT_STR_EQ(" << #actual << ", " << #expected << ") failed\n"; \
            std::cerr << "         Actual:   \"" << act << "\"\n"; \
            std::cerr << "         Expected: \"" << exp << "\"\n"; \
            current_test_failed = true; \
            failed_assertions++; \
        } \
    } while (0)

// Helper to define and run a test case
typedef void (*test_func_t)();

struct TestCase {
    std::string name;
    test_func_t func;
};

inline void run_test_case(const TestCase& tc, int& passed_tests, int& total_tests) {
    total_tests++;
    current_test_failed = false;
    std::cout << "[ RUN      ] " << tc.name << "\n";
    try {
        tc.func();
    } catch (const std::exception& e) {
        std::cerr << "  [\033[31mFAIL\033[0m] Unhandled exception: " << e.what() << "\n";
        current_test_failed = true;
        failed_assertions++;
    } catch (...) {
        std::cerr << "  [\033[31mFAIL\033[0m] Unknown unhandled exception\n";
        current_test_failed = true;
        failed_assertions++;
    }
    
    if (current_test_failed) {
        std::cout << "[  \033[31mFAILED\033[0m  ] " << tc.name << "\n";
    } else {
        std::cout << "[       \033[32mOK\033[0m ] " << tc.name << "\n";
        passed_tests++;
    }
}

#endif // TEST_FRAMEWORK_H
