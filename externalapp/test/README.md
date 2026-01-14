# Unit Testing - Using GTest and CMOCK

This document describes how to perform unit testing of C modules  using **GoogleTest** and **CMOCK** (Google Mock Extension for C Functions).

## Overview

**GoogleTest (GTest)** is an open-source **C++ unit testing framework** developed by Google.

**CMOCK** is a mocking framework designed specifically to **mock C functions**. This is just a set of headers providing a way to use tools for mock methods with mock functions in tests.

CMOCK headers are available under `test/lib/` directory.

### Why CMOCK ?

GoogleTest alone cannot mock free C functions.

CMOCK fills this gap by allowing:

* Mocking **global C functions**
* Controlling return values and output parameters
* Verifying function call expectations

CMOCK works by **replacing the real C function at link time** using a generated mock object.

## Tools & Requirements

### System Requirements

* GNU/Linux platform that Google Mock supports

### Software Requirements

* GCC / G++
* GoogleTest & Google Mock
* CMOCK
* Build Tools (Make / CMake)
* Code Coverage Tools (gcov / lcov)

### Installation & Prerequisites

1. GCC / G++

   ```
   sudo apt update
   sudo apt install -y build-essential
   ```
2. GoogleTest & Google Mock

   ```
   sudo apt update
   sudo apt install googletest libgtest-dev libgmock-dev
   ```
3. CMOCK

   ```
   git clone https://github.com/hjagodzinski/C-Mock.git
   cd C-Mock
   sudo make install
   ```
4. Build Tools (Make / CMake)

   ```
   sudo apt install -y make cmake
   ```
5. Code Coverage Tools (gcov / lcov)

   ```
   sudo apt install -y gcovr lcov
   ```

## Build Steps

Run the script `runtest.sh` in test/ directory for building the test files with coverage report using below command:

```
 ./runtest.sh sourcefile.c
```

Example:

```
 ./runtest.sh helloworld.c
```

The build artifacts will be generated in the `test/driver/sample/` directory.

To see detailed **coverage metrics** for the source files, browse directly to `test/driver/sample/report/index.html`

Expected output:

```
[----------] Global test environment tear-down
[==========] 7 tests from 2 test suites ran. (0 ms total)
[  PASSED  ] 7 tests.

Overall coverage rate:
lines......: 100.0% (6 of 6 lines)
functions......: 100.0% (2 of 2 functions)
```

**Note:** For the unit testing of Static functions, their function prototype without static keyword must be added in the test case file, here, in `test_helloworld.cc`

Example :

For the unit testing of static function `divide(int a, int b)`

```
#include <cmock/cmock.h>

extern "C" {
    // Includes of Source code
    #include <helloworld.h>
    #include <helper.h>

    // Static function prototypes for testing
    int32_t divide(int32_t lDividend, int32_t lDivisor);
}
```

### Makefile Modifications Required :

#### Update Source Files Under Test

Modify the list of **C source files** belonging to the unit under test, and Replace with your modules source files:

```
 SRC_IMPL := $(UNIT_DIR)/helloworld.c
```

#### Update Test Source File

Specify the correct test file name:

```
 SRC_TEST := test_helloworld.cc
```

Example:

```
 TEST_SRCS = test_ModuleName.cc
```

#### Update Include Paths (If Needed)

Default include paths:

```
 INC := -I$(UNIT_DIR)/ -I$(TEST_DIR)/
```

## Test Case Development Guide

* Unit test cases are written in **C++ source files** with the `.cc` extension.
* Each test file must follow the naming convention **`test_<module_name>.cc`**.
* The test file should be placed under the `test/` directory, following the **same folder hierarchy as the source code**. For example, if the source code is located in `source/driver/`, the corresponding test file should be placed in `test/driver/`
* Test cases are compiled using the **`g++` compiler**.
* A **Makefile** is used to build the tests, ensuring that all required **CMOCK and GoogleTest dependencies** are correctly included and linked.

### Why Mocking Is Needed ?

In embedded or system-level software, functions often depend on:

* Hardware drivers
* OS services
* External libraries

During unit testing, these dependencies make it difficult to achieve **complete code coverage**. To address this, dependent functions are **stubbed or mocked**.

Mocking of these lower-level hardware functions allows :

* Replace real implementations at test time
* Control return values and output parameters

### Test Case Using GTest

Below shows a sample test case for the function `add(int a, int b)` using GTest (without mocking)

```
TEST(AdditionTest, TwoPositiveNumbers) 
{
    // Expect that the call to add(1, 2) is equal to 3
    EXPECT_EQ(3, add(1, 2));
}
```

### Test Case Using CMOCK

Below shows how the function `isDivisible(int a, int b)` is mocked to control its return value and output parameters when it is invoked from `divide(int a, int b)`.

#### Test File Structure

##### Includes

```test_sensorControl.cc
#include <cmock/cmock.h>

extern "C" {
 // Add your source code headers

 // Static function prototypes

}
```

`extern "C"` prevents C++ name mangling while including source code headers.

#### Declaring and Implementing Mocks

Syntax:

```
DECLARE_FUNCTION_MOCK<N>(MockClassName, FunctionName, ReturnType(Args...));
```

Here, `N` Indicates the number of function arguments

Example:

```
DECLARE_FUNCTION_MOCK1(IsDivisibleMock, isDivisible, bool(int32_t));
```

#### Implementing Function Mocks

Syntax:

```
 IMPLEMENT_FUNCTION_MOCK<N>(MockClassName, FunctionName, ReturnType(Args...));
```

Here, `N` Indicates the number of function arguments

Example:

```
IMPLEMENT_FUNCTION_MOCK1(IsDivisibleMock, isDivisible, bool(int32_t));
```

#### Writing Test Cases Using CMOCK

##### Creating a Mock Object

```
 IsDivisibleMock checkDivisibleMock;
```

* Mocks are **scoped objects**.
* While the object exists → the real function is replaced.
* When destroyed → real function is restored.

##### Setting Expectations

```
EXPECT_FUNCTION_CALL(checkDivisibleMock, (5))
        .WillOnce(Return(true));
```

* Verifies the function is called
* Controls return value

##### Mocking Output Parameters

```
 EXPECT_FUNCTION_CALL(checkDivisibleMock, (testing::_))
    .WillOnce(DoAll(
        testing::SetArgPointee<0>(2),
        Return(true)
    ));
```

* `SetArgPointee<0>` sets  output parameter
* `Return(true)` controls function result

#### Example Test Case Using CMOCK :

```
TEST(DivisionTest, TwoPositiveNumbers) 
{
    IsDivisibleMock checkDivisibleMock;

    // The function isDivisible() is mocked to return true
    EXPECT_FUNCTION_CALL(checkDivisibleMock, (5))
        .WillOnce(Return(true));

    // Expect that the call to divide(10, 5) is equal to 2
    EXPECT_EQ(2, divide(10, 5));
}
```
