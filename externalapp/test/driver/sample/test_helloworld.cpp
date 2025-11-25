// helloworld_test.cpp

#include "helloworld.h"
#include <gtest/gtest.h>

// Define a test suite called 'AdditionTest'
// and a specific test case within it called 'TwoPositiveNumbers'
TEST(AdditionTest, TwoPositiveNumbers) {
    // Expect that the call to add(1, 2) is equal to 3
    EXPECT_EQ(3, add(1, 2));
}

// Another test case
TEST(AdditionTest, OnePositiveAndOneNegative) {
    // Expect that the call to add(10, -5) is equal to 5
    EXPECT_EQ(5, add(10, -5));
}

// The main function that runs all tests
int main(int argc, char **argv) {
    // Initializes the Google Test framework
    ::testing::InitGoogleTest(&argc, argv);
    // Runs all registered tests
    return RUN_ALL_TESTS();
}