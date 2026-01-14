/**
 * \internal \copyright
 * Copyright (c) 2025 Trenser Technology Solutions (P) Ltd.
*/

/**
 * 
 * \internal
 * modification history
 * 31Dec25,vrk  Updated coding standard & style.
 * 29Nov25,vrk  Created, and added the includes, and function definitions.
*/

/**
 * ### Description
 * 
 * This file contains includes, and function definitions related to add, divide
 * two integers.
 * 
*/

/* includes */

#include "helloworld.h"
#include "helper.h"

/* forward declarations */

static int32_t divide(int32_t lDividend, int32_t lDivisor);

/*******************************************************************************
 * divide - Perform division of two integers.
 * 
 * DESCRIPTION:
 * This function performs integer division of dividend by divisor. Returns
 * quotient if divisor is valid (non-zero), otherwise returns 0 as error case.
 * 
 * PARAMETERS:
 * <lDividend>
 * [in] int32_t [-2^31 to 2^31-1] -- Dividend value for division operation
 * 
 * <lDivisor>
 * [in] int32_t [-2^31 to 2^31-1, != 0] -- Divisor value (must be non-zero)
 * 
 * GLOBALS: N/A
 * 
 * DESIGN GLOBALS: N/A
 * 
 * RETURNS: 
 * <int32_t>
 * Quotient (lDivident / lDivisor) if divisor valid, else 0.
 * 
 * ERRNO: N/A
 * 
 * SEE ALSO: isDivisible()
*/

static int32_t divide
    (
    int32_t lDividend, 
    int32_t lDivisor
    )
    {

    if (isDivisible(lDivisor))
        {
        return lDividend / lDivisor;
        }
    else
        {
        return 0;
        }
    }

/*******************************************************************************
 * add - Perform addition of two integers.
 * 
 * DESCRIPTION:
 * This function adds two 32-bit signed integers and returns their sum.
 * 
 * PARAMETERS:
 * <lNum1>
 * [in] int32_t [-2^31 to 2^31-1] -- First number for addition
 * 
 * <lNum2>
 * [in] int32_t [-2^31 to 2^31-1] -- Second number for addition
 * 
 * GLOBALS: N/A
 * 
 * DESIGN GLOBALS: N/A
 * 
 * RETURNS: 
 * <int32_t>
 * Sum of lNum1 + lNum2.
 * 
 * ERRNO: N/A
 * 
 * SEE ALSO: N/A
*/

int32_t add
    (
    int32_t lNum1, 
    int32_t lNum2
    ) 
    {

    // A simple function that returns the sum of two numbers.
    return lNum1 + lNum2;
    }
