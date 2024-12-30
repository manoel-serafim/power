/**
 * @file       main.c
 * @brief      Demonstrates modular exponentiation using sliding window technique.
 * @details    This example shows the usage of the sliding window modular exponentiation 
 *             function to calculate large modular powers efficiently. The program is set 
 *             up with example values for base, exponent, and modulus to verify the 
 *             implementation.
 *
 * @copyright  (C) 2024, Manoel Augusto de Souza Serafim
 *             All rights reserved.
 *
 * @author     Manoel Serafim
 * @email      manoel.serafim@proton.me
 * @date       2024-12-25
 * @github     https://github.com/manoel-serafim
 *
 */

#include "power.h"
#include "fast_memory.h"
#include <stdint.h>
//NOLINTNEXTLINE(llvmlibc-restrict-system-libc-headers)
#include <stdio.h>



// Example helper to print a large number
void print_large(const uint8_t *number, uint32_t length) {
    //NOLINTNEXTLINE(altera-unroll-loops)
    for (uint32_t i = 0; i < length; ++i) {
        printf("%02X", number[i]);
    }
    printf("\n");
}

int main(void) {
    // Example inputs
    uint8_t base[BYTE_SIZE_VARS] = {0};
    fast_memset(base, 0, BYTE_SIZE_VARS);

    uint8_t exponent[BYTE_SIZE_VARS] = {0};
    fast_memset(base, 0, BYTE_SIZE_VARS);

    uint8_t modulus[BYTE_SIZE_VARS] = {0};
    fast_memset(base, 0, BYTE_SIZE_VARS);

    uint8_t result[BYTE_SIZE_VARS] = {0};
    fast_memset(base, 0, BYTE_SIZE_VARS);


    // Initialize base, exponent, and modulus with example values
    
    base[BYTE_SIZE_VARS - 1] = 5; // Example: base = 5
    exponent[BYTE_SIZE_VARS - 1] = 2; // Example: exponent = 3
    modulus[BYTE_SIZE_VARS - 1] = 7; // Example: modulus = 7

    // Perform sliding window modular exponentiation
    sliding_window_modular_exponentiation(result, base, exponent, 8, modulus);

    // Print results
    printf("Base: ");
    print_large(base, BYTE_SIZE_VARS);

    printf("Exponent: ");
    print_large(exponent, BYTE_SIZE_VARS);

    printf("Modulus: ");
    print_large(modulus, BYTE_SIZE_VARS);

    printf("Result: ");
    print_large(result, BYTE_SIZE_VARS);

    return 0;
}
