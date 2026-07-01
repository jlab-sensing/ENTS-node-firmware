/**
 * @file example_edu0157.c
 *
 * Prints the raw string returned by the EDU0157 weather station.
 * based off bme280_example
 */

// stdlib includes
#include <stdio.h>
#include <string.h>

// system includes
#include "board.h"
#include "gpio.h"
#include "i2c.h"
#include "main.h"
#include "sys_app.h"
#include "usart.h"
#include "stm32_systime.h"

// user includes
#include "EDU0157_sensor.h"

#ifndef DELAY
#define DELAY 1000
#endif

HAL_StatusTypeDef rc;

int main(void)
{
    /* Reset peripherals and initialize HAL */
    HAL_Init();

    /* Configure system clock */
    SystemClock_Config();

    /* Initialize system app */
    SystemApp_Init();

    /* Initialize peripherals */
    MX_GPIO_Init();
    MX_I2C1_Init();
    MX_USART2_UART_Init();
    APP_LOG(TS_OFF, VLEVEL_ALWAYS, "HELLO\r\n");
    /* Initialize EDU0157 sensor */
    if (!EDU0157Init())
    {
        APP_LOG(TS_OFF, VLEVEL_ALWAYS,
                "EDU0157 init failed\r\n");

        while (1);
    }

    APP_LOG(TS_OFF, VLEVEL_ALWAYS,
            "Example EDU0157 (%s), compile on %s %s\r\n",
            __FILE__, __DATE__, __TIME__);

    while (1)
    {
        HAL_Delay(DELAY);

        uint8_t buffer[64] = {0};

        size_t len = EDU0157Measure(
            buffer,
            SysTimeGet(),
            0
        );

        if (len == 0)
        {
            APP_LOG(TS_OFF, VLEVEL_ALWAYS,
                    "EDU0157 Measure Failed\r\n");
            continue;
        }

        /* Ensure null termination for printing */
        if (len < sizeof(buffer))
        {
            buffer[len] = '\0';
        }
        else
        {
            buffer[sizeof(buffer) - 1] = '\0';
        }

        APP_LOG(TS_OFF, VLEVEL_ALWAYS,
                "EDU0157 RAW: %s\r\n",
                (char *)buffer);
    }
}