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
 * This file contains includes, and function definitions related to divisibility
 * check.
 * 
*/

/* includes */

#include "helper.h"

/*******************************************************************************
 * isDivisible - Check if divisor is valid for division operation.
 * 
 * DESCRIPTION:
 * This function validates a divisor for division operations
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
 * <true>
 * if lDivisor is valid (non-zero)
 * 
 * <false>
 * if lDivisor is zero (invalid for division)
 * 
 * ERRNO: N/A
 * 
 * SEE ALSO: N/A
*/

bool isDivisible
    (
    int32_t lDivisor
    )
    {

    if (0 == lDivisor)
        {
        return false;
        }

    return true;
    }
