//************************* Test_SensorControl *********************************
// Copyright (c) 2026 Trenser Technology Solutions (P) Ltd
// All Rights Reserved
//******************************************************************************
//
// File    : test_helloworld.cc
// Summary : Test cases for helloworld module
// Note    : 
// Author  : Aromal
// Date    : 31-December-2025
//
//******************************************************************************
//*************************** Include Files ************************************
#include <cmock/cmock.h>

extern "C" {
    // Includes of Source code
    #include <helloworld.h>
    #include <helper.h>

    // Static function prototypes for testing
    int32_t divide(int32_t lDividend, int32_t lDivisor);
}

using namespace ::testing;

//*************************** Local Constants **********************************

//************************* Class Declarations *********************************
DECLARE_FUNCTION_MOCK1(IsDivisibleMock, isDivisible, bool(int32_t));

//************************* Class Implementation *******************************
IMPLEMENT_FUNCTION_MOCK1(IsDivisibleMock, isDivisible, bool(int32_t));

//****************************** Test Code *************************************
//**************************** AdditionTest ************************************
// Purpose : Verifies that the FUT returns sum of [lNum1] + [lNum2]
// Inputs  : N/A
// Outputs : N/A
// Return  : PASSED : if the add(1, 2) returns expected result (3)
//         : FAILED : if the add(1, 2) returns unexpected result (!= 3)
// Notes   : None
//******************************************************************************
TEST(AdditionTest, TwoPositiveNumbers) 
{
    // Expect that the call to add(1, 2) is equal to 3
    EXPECT_EQ(3, add(1, 2));
}

//**************************** AdditionTest ************************************
// Purpose : Verifies that the FUT returns sum of [lNum1] + [lNum2]
// Inputs  : N/A
// Outputs : N/A
// Return  : PASSED : if the add(10, -5) returns expected result (5)
//         : FAILED : if the add(10, -5) returns unexpected result (!= 5)
// Notes   : None
//******************************************************************************
TEST(AdditionTest, OnePositiveAndOneNegative) 
{
    // Expect that the call to add(10, -5) is equal to 5
    EXPECT_EQ(5, add(10, -5));
}

//**************************** DivisionTest ************************************
// Purpose : Verifies that the FUT correctly returns quotient when divisor is
//         : valid
// Inputs  : N/A
// Outputs : N/A
// Return  : PASSED : if the divide(10, 5) returns expected result (2)
//         : FAILED : if the divide(10, 5) returns unexpected result (!= 2)
// Notes   : Mocks isDivisible(5) to return true to test successful division
//******************************************************************************
TEST(DivisionTest, TwoPositiveNumbers) 
{
    IsDivisibleMock checkDivisibleMock;

    // The function isDivisible() is mocked to return true
    EXPECT_FUNCTION_CALL(checkDivisibleMock, (5))
        .WillOnce(Return(true));

    // Expect that the call to divide(10, 5) is equal to 2
    EXPECT_EQ(2, divide(10, 5));
}

//**************************** DivisionTest ************************************
// Purpose : Verifies that the FUT correctly returns quotient when divisor is
//         : valid
// Inputs  : N/A
// Outputs : N/A
// Return  : PASSED : if the divide(-10, -5) returns expected result (2)
//         : FAILED : if the divide(-10, -5) returns unexpected result (!= 2)
// Notes   : Mocks isDivisible(-5) to return true to test successful division
//******************************************************************************
TEST(DivisionTest, TwoNegativeNumbers) 
{
    IsDivisibleMock checkDivisibleMock;

    // The function isDivisible() is mocked to return true
    EXPECT_FUNCTION_CALL(checkDivisibleMock, (-5))
        .WillOnce(Return(true));

    // Expect that the call to divide(-10, -5) is equal to 2
    EXPECT_EQ(2, divide(-10, -5));
}

//**************************** DivisionTest ************************************
// Purpose : Verifies that the FUT correctly returns quotient when divisor is
//         : valid
// Inputs  : N/A
// Outputs : N/A
// Return  : PASSED : if the divide(10, -5) returns expected result (-2)
//         : FAILED : if the divide(10, -5) returns unexpected result (!= -2)
// Notes   : Mocks isDivisible(-5) to return true to test successful division
//******************************************************************************
TEST(DivisionTest, OnePositiveAndOneNegative) 
{
    IsDivisibleMock checkDivisibleMock;

    // The function isDivisible() is mocked to return true
    EXPECT_FUNCTION_CALL(checkDivisibleMock, (-5))
        .WillOnce(Return(true));

    // Expect that the call to divide(10, -5) is equal to -2
    EXPECT_EQ(-2, divide(10, -5));
}

//**************************** DivisionTest ************************************
// Purpose : Verifies that the FUT correctly returns quotient when divisor is
//         : valid
// Inputs  : N/A
// Outputs : N/A
// Return  : PASSED : if the divide(10, 3) returns expected result (3)
//         : FAILED : if the divide(10, 3) returns unexpected result (!= 3)
// Notes   : Mocks isDivisible(3) to return true to test successful division
//******************************************************************************
TEST(DivisionTest, NumsWithLeavingRemainder) 
{
    IsDivisibleMock checkDivisibleMock;

    // The function isDivisible() is mocked to return false
    EXPECT_FUNCTION_CALL(checkDivisibleMock, (3))
        .WillOnce(Return(true));

    // Expect that the call to divide(10, 3) is equal to 0
    EXPECT_EQ(3, divide(10, 3));
}

//**************************** DivisionTest ************************************
// Purpose : Verifies that the FUT correctly returns 0 when divisor is not
//         : valid
// Inputs  : N/A
// Outputs : N/A
// Return  : PASSED : if the divide(10, 0) returns expected result (0)
//         : FAILED : if the divide(10, 0) returns unexpected result (!= 0)
// Notes   : Mocks isDivisible(0) to return false to test successful division
//******************************************************************************
TEST(DivisionTest, DivisorWithValueZero) 
{
    IsDivisibleMock checkDivisibleMock;

    // The function isDivisible() is mocked to return false
    EXPECT_FUNCTION_CALL(checkDivisibleMock, (0))
        .WillOnce(Return(false));

    // Expect that the call to divide(10, 0) is equal to 0
    EXPECT_EQ(0, divide(10, 0));
}

//******************************** main ****************************************
// Purpose : Entry point for GoogleTest framework execution
// Inputs  : argc : Command line arguments
// Inputs  : argv : Command line arguments
// Outputs : N/A
// Return  : 0          : All tests passed
//         : Non-zero   : One or more tests failed
// Notes   : Initializes and runs all registered test cases
//******************************************************************************
int main(int argc, char **argv) 
{
    // Initializes the Google Test framework
    ::testing::InitGoogleTest(&argc, argv);

    // Runs all registered tests
    return RUN_ALL_TESTS();
}
