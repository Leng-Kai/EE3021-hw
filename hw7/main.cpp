#include "mbed.h"

// Sensors drivers present in the BSP library
#include "stm32l475e_iot01_tsensor.h"
#include "stm32l475e_iot01_hsensor.h"
#include "stm32l475e_iot01_psensor.h"
#include "stm32l475e_iot01_magneto.h"
#include "stm32l475e_iot01_gyro.h"
#include "stm32l475e_iot01_accelero.h"

#define SEMIHOSTING
#include <math.h>
#include "arm_math.h"
#if defined(SEMIHOSTING)
#include <stdio.h>
#endif
/* ----------------------------------------------------------------------
* Defines each of the tests performed
* ------------------------------------------------------------------- */
#define MAX_BLOCKSIZE   8
#define DELTA           (0.000001f)
/* ----------------------------------------------------------------------
* Declare I/O buffers
* ------------------------------------------------------------------- */
float32_t wire1[MAX_BLOCKSIZE];
float32_t wire2[MAX_BLOCKSIZE];
float32_t wire3[MAX_BLOCKSIZE];
/* ----------------------------------------------------------------------
* Test input data for Floating point Variance example for 32-blockSize
* Generated by the MATLAB randn() function
* ------------------------------------------------------------------- */
float32_t testInput_f32[8] =
{
  -0.432564811528221,  -1.665584378238097,   0.125332306474831,   0.287676420358549,
  -1.146471350681464,   1.190915465642999,   1.189164201652103,  -0.037633276593318,
};
/* ----------------------------------------------------------------------
* Declare Global variables
* ------------------------------------------------------------------- */
uint32_t blockSize = 8;
float32_t  refVarianceOut = 0.903941793931839;
/* ----------------------------------------------------------------------
* Variance calculation test
* ------------------------------------------------------------------- */

DigitalOut led(LED1);

int main()
{
  arm_status status;
  float32_t mean, oneByBlockSize;
  float32_t variance;
  float32_t diff;
  status = ARM_MATH_SUCCESS;
#if defined(SEMIHOSTING)
  printf("START\n");
#endif

    float sensor_value = 0;
    int16_t pDataXYZ[3] = {0};
    float pGyroDataXYZ[3] = {0};

    printf("Start sensor init\n");

    BSP_TSENSOR_Init();
    BSP_HSENSOR_Init();
    BSP_PSENSOR_Init();

    BSP_MAGNETO_Init();
    BSP_GYRO_Init();
    BSP_ACCELERO_Init();
    
    int cur = 0;

    while(1) {

        BSP_GYRO_GetXYZ(pGyroDataXYZ);

        testInput_f32[cur++] = pGyroDataXYZ[0];
        cur &= 7;

        /* Calculation of mean value of input */
        /* x' = 1/blockSize * (x(0)* 1 + x(1) * 1 + ... + x(n-1) * 1) */
        /* Fill wire1 buffer with 1.0 value */
        arm_fill_f32(1.0,  wire1, blockSize);
        /* Calculate the dot product of wire1 and wire2 */
        /* (x(0)* 1 + x(1) * 1 + ...+ x(n-1) * 1) */
        arm_dot_prod_f32(testInput_f32, wire1, blockSize, &mean);
        /* Calculation of 1/blockSize */
        oneByBlockSize = 1.0 / (blockSize);
        /* 1/blockSize * (x(0)* 1 + x(1) * 1 + ... + x(n-1) * 1)  */
        arm_mult_f32(&mean, &oneByBlockSize, &mean, 1);
        printf("mean: %.2f\n", mean);
        /* Calculation of variance value of input */
        /* (1/blockSize) * (x(0) - x') * (x(0) - x') + (x(1) - x') * (x(1) - x') + ... + (x(n-1) - x') * (x(n-1) - x') */
        /* Fill wire2 with mean value x' */
        arm_fill_f32(mean,  wire2, blockSize);
        /* wire3 contains (x-x') */
        arm_sub_f32(testInput_f32, wire2, wire3, blockSize);
        /* wire2 contains (x-x') */
        arm_copy_f32(wire3, wire2, blockSize);
        /* (x(0) - x') * (x(0) - x') + (x(1) - x') * (x(1) - x') + ... + (x(n-1) - x') * (x(n-1) - x') */
        arm_dot_prod_f32(wire2, wire3, blockSize, &variance);
        /* Calculation of 1/blockSize */
        oneByBlockSize = 1.0 / (blockSize - 1);
        /* Calculation of variance */
        arm_mult_f32(&variance, &oneByBlockSize, &variance, 1);
        /* absolute value of difference between ref and test */
        diff = fabsf(refVarianceOut - variance);
        /* Comparison of variance value with reference */

        printf("variance: %.2f\n", variance);

        if (variance > 50000000) {
            led = 1;
        } else {
            led = 0;
        }

    }
}