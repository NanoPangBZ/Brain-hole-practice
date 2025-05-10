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
        HAL_Delay( 600 );
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

int user_main(void* args)
{
    ERR_CHECK( qspi_flash_init() );
    ERR_CHECK( qspi_erase_write_test() );

    ok_led_show();

    return -1;
}

