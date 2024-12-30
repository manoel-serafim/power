#include "power.h"
#include "fast_memory.h"
#include <stdint.h>

// Helper function to compare two large numbers
static uint32_t compare_large(const uint8_t *value_a,
                         const uint8_t *value_b
                         )
{
    //NOLINTNEXTLINE(altera-unroll-loops)
    for (uint32_t i = 0; i < BYTE_SIZE_VARS; ++i)
    {
        if (value_a[i] > value_b[i])
        {
            return 1;
        }
        if (value_a[i] < value_b[i])
        {
            return 2;
        }
    }
    return 0;
}
//NOLINTNEXTLINE(llvmlibc-restrict-system-libc-headers)
#include <stdio.h>
void print_larged(const uint8_t *number, uint32_t length) {
    //NOLINTNEXTLINE(altera-unroll-loops)
    for (uint32_t i = 0; i < length; ++i) {
        printf("%02X", number[i]);
    }
    printf("\n");
}
static void subtract_large(uint8_t *result,
                           const uint8_t *value_a,
                           const uint8_t *value_b
                           )
{
    uint32_t borrow = 0;
    uint32_t len = BYTE_SIZE_VARS;


    uint32_t diff = 0;
    //NOLINTNEXTLINE(altera-unroll-loops)
    do
    {
        --len;
        diff = (uint32_t)value_a[len] - (uint32_t)value_b[len] - borrow;  // Make sure to treat as unsigned

        // Check if a borrow occurred
        borrow = (diff > 0xFFU) ? 1 : 0;

        // Store the result with the borrow corrected
        result[len] = (uint8_t)(diff & 0xFFU);  // Ensure we store the result in the lower 8 bits
        
    }
    while(len > 0);
}

// Modular reduction: result = a % mod
static void modular_reduce(uint8_t *result,
                                 const uint8_t *value_a,
                                 const uint8_t *mod
                                 )
{
    uint8_t temp[BYTE_SIZE_VARS];
    fast_memcpy(temp, &value_a[BYTE_SIZE_VARS], BYTE_SIZE_VARS);
    
    //NOLINTNEXTLINE(altera-unroll-loops)
    while (compare_large(temp, mod) != 2)
    {
        subtract_large(temp, temp, mod);
    }
    fast_memcpy(result, temp, BYTE_SIZE_VARS);
}

// Modular multiplication: result = (a * b) % mod
static void modular_multiply(uint8_t *result,
                             const uint8_t *value_a,
                             const uint8_t *value_b,
                             const uint8_t *mod
                             )
{
    uint8_t temp[2 * BYTE_SIZE_VARS];
    fast_memset(temp, 0, 2 * BYTE_SIZE_VARS);
    static const uint16_t BYTE_ENABLER = 0xFFU;
    static const uint16_t BYTE_SHIFT_AMOUNT = 8U;
    for (uint32_t i = 0; i < BYTE_SIZE_VARS; ++i)
    {
        //NOLINTNEXTLINE(altera-unroll-loops)
        for (uint32_t j = 0; j < BYTE_SIZE_VARS; ++j)
        {
            uint32_t index = (2 * BYTE_SIZE_VARS - 1) - (i + j);
            uint16_t prod = (uint16_t) value_a[BYTE_SIZE_VARS - 1 - i] * value_b[BYTE_SIZE_VARS - 1 - j];
            temp[index] += prod & BYTE_ENABLER;
            temp[index - 1] += (prod >> BYTE_SHIFT_AMOUNT) + (temp[index] < (prod & BYTE_ENABLER));
        }
    }

    modular_reduce(result, temp, mod);
}


// Sliding Window Modular Exponentiation for large integers
void sliding_window_modular_exponentiation(
    uint8_t * const result,
    const uint8_t *base,
    const uint8_t *exponent,
    uint32_t exponent_bit_lenght,
    const uint8_t *modulo
    )
{

    /**
     * @section 1. Precomputation
     * @details Prepares reusable multiplies to be added in each window.
     *          The amount of computations is determined by the window size 2^(WINDOW_SIZE-1).
     */
    #define QUANT_PRECOMPUTE_WINDOWS 64U

    uint8_t precomputed[QUANT_PRECOMPUTE_WINDOWS][BYTE_SIZE_VARS];


    uint8_t holder[BYTE_SIZE_VARS];

    // g₂ = base²
    modular_multiply(holder, base, base, modulo);

    // g₃ = g₁ * g₂ = precomputed[1]
    modular_multiply(precomputed[1], base, holder, modulo);

    //g(2i+1) = g(2i-1) * g₂ = precomputed[i]
    uint8_t precompute_index = 2U;
    //NOLINTNEXTLINE(altera-unroll-loops)
    do
    {
        modular_multiply(precomputed[precompute_index], precomputed[precompute_index-1U], holder, modulo);
        ++precompute_index;
    }
    while (precompute_index < QUANT_PRECOMPUTE_WINDOWS);



    //g₀ = base¹
    fast_memcpy(precomputed[0], base, BYTE_SIZE_VARS);

    /**
     * @section 2. PreLoop
     * @details Prepares holder variables
     */
    //result = 1
    fast_memset((void *)result, 0U, BYTE_SIZE_VARS-1U);
    result[BYTE_SIZE_VARS-1U] = 1U;


    //set up index variables
    //Considering it BE left-to-right
    int32_t exponent_bit_index = (int32_t)(BYTE_SIZE_VARS<<3U) -(int32_t)exponent_bit_lenght ;
    int32_t window_start = 0;
    uint8_t window_value = 0U;
    uint32_t window_bit_index = 0U;

    /**
     * @section 3. Find and process windows
     */
    do
    {
        //Find Window
        static const uint8_t LAST_3BIT_ENABLER = 0x7U;
        if (((uint8_t)(exponent[(uint32_t)exponent_bit_index >> 3U] >> (uint8_t)( LAST_3BIT_ENABLER - (int8_t)((uint32_t)exponent_bit_index & LAST_3BIT_ENABLER))) & 1U) == 1U)    //search for window
        {
            //Process Window
            static const int32_t WINDOW_BIT_START_OFFSET = 6U;
            window_start = exponent_bit_index + WINDOW_BIT_START_OFFSET;

            if (window_start >= (BYTE_SIZE_VARS<<3U))
            {
                window_start = (BYTE_SIZE_VARS<<3U)-1;
            }

            //NOLINTNEXTLINE(altera-unroll-loops, altera-id-dependent-backward-branch)
            for (window_bit_index = (uint32_t)window_start; window_bit_index >= (uint32_t)exponent_bit_index; --window_bit_index)
            {
                //NOLINTNEXTLINE(readability-suspicious-call-argument)
                modular_multiply(holder, result, result, modulo);
                fast_memcpy(result, holder, BYTE_SIZE_VARS);

                window_value = (uint8_t)(window_value << 1U) | ((uint8_t)(exponent[window_bit_index >> 3U] >> (uint8_t)( LAST_3BIT_ENABLER - (int8_t)(window_bit_index & LAST_3BIT_ENABLER))) & 1U);
            }
            //NOLINTNEXTLINE(readability-suspicious-call-argument)
            modular_multiply(holder, result, result, modulo);
            fast_memcpy(result, holder, BYTE_SIZE_VARS);

            
            //NOLINTNEXTLINE(readability-suspicious-call-argument)
            modular_multiply(holder, result, precomputed[window_value>>1U], modulo);
            fast_memcpy(result, holder, BYTE_SIZE_VARS);

            exponent_bit_index = window_start+1;

        }
        else // not a window enables us to go to the next bit...
        {
            //NOLINTNEXTLINE(readability-suspicious-call-argument)
            modular_multiply(holder, result, result,  modulo);
            fast_memcpy(result, holder, BYTE_SIZE_VARS);
            ++exponent_bit_index;
        }

    }
    //NOLINTNEXTLINE(altera-id-dependent-backward-branch)
    while (exponent_bit_index < (BYTE_SIZE_VARS<<3U));
}
