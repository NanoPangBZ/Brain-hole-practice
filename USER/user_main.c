#include <stdio.h>
#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_gpio.h"

#include "quadspi.h"

#include "qspi_flash.h"

static void qspi_init(void)
{
    MX_QUADSPI_Init();
}

int user_main(void* args)
{
    qspi_init();
    qspi_flash_init();
    while(1)
    {
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13); // Toggle the state of pin GPIOB_PIN_0
        HAL_Delay( 1000 );
    }
    return -1;
}

