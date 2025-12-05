/* CM4/Core/Inc/test_suite.h */
#ifndef UNIT_TESTS_H
#define UNIT_TESTS_H

// Only define the runner if we are in DEBUG mode
#ifdef DEBUG
    void Run_Boot_Tests(void);
#endif

#endif // TEST_SUITE_H
