#include <stdio.h>
#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_gpio.h"

#include "quadspi.h"

#include "qspi_flash.h"

#define ERR_CHECK( ret )    if( ret ) { err_led_show(); }

static void err_led_show(void)
{
    while(1)
    {
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13); // Toggle the state of pin GPIOB_PIN_0
        HAL_Delay( 200 );
    }
}

static void ok_led_show(void)
{
    while(1)
    {
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13); // Toggle the state of pin GPIOB_PIN_0
        HAL_Delay( 100 );
    }
}

static int qspi_erase_write_test(void)
{
    uint8_t programe_data[ 1024 ];
    uint8_t read_data[1024];

    for( uint16_t temp = 0 ; temp < 1024 ; temp++ )
    {
        programe_data[ temp ] = temp;
    }

    for( uint8_t temp = 0 ; temp < 64 ; temp++ )
    {
        programe_data[ temp ] = 0x5A;
    }

    qspi_flash_erase( 0 , 4 * 1024 );
    qspi_flash_programe( 0 , programe_data , 1024 );
    qspi_flash_read( 0 , read_data , 1024 );

    for( uint16_t temp = 0 ; temp < 1024 ; temp++ )
    {
        if( programe_data[ temp ] != read_data[ temp ] )
        {
            return -1;
        }
    }

    return 0;
}

static int qspi_flash_download_test(void)
{
    qspi_flash_init();
    uint8_t read_data[256];
    qspi_flash_read( 0 , 0 , 256 );

    ok_led_show();
}

int user_main(void* args)
{
    qspi_flash_init();
    qspi_flash_enter_mem_map_mode();

    ok_led_show();

    return -1;
}

