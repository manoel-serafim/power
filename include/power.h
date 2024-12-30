/**
 * @file       power.h
 * @brief      Modular exponentiation using the sliding window technique.
 * @details    This file declares the function for performing modular exponentiation 
 *             using an optimized sliding window algorithm. The implementation is 
 *             designed to handle large integers efficiently, making it suitable for 
 *             cryptographic applications such as RSA encryption, digital signatures, 
 *             and other scenarios requiring modular exponentiation.
 *
 *             The sliding window technique reduces the number of modular multiplications 
 *             by precomputing powers of the base, resulting in improved performance for 
 *             exponentiation with large bit-lengths.
 *
 * @copyright (C) 2024, Manoel Augusto de Souza Serafim
 *             All rights reserved.
 *
 * @author     Manoel Serafim
 * @email      manoel.serafim@proton.me
 * @date       2024-12-25
 * @github     https://github.com/manoel-serafim
 *
 */

#ifndef POWER_H
#define POWER_H

#include <stdint.h>

#ifndef BYTE_SIZE_VARS
    #define BYTE_SIZE_VARS 64U
#endif

void sliding_window_modular_exponentiation(
    uint8_t *result,
    const uint8_t *base,
    const uint8_t *exponent,
    uint32_t exponent_bit_lenght,
    const uint8_t *modulo
    );

#endif //POWER_H
