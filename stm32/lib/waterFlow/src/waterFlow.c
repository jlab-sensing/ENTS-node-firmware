/**
 ******************************************************************************
* @file    waterFlow.c
* @author  Caden Jacobs
*
* @brief   This library is designed to read measurements from a water Flow
*          sensor from DFRobot.
*          https://wiki.dfrobot.com/Gravity__Water_Flow_Sensor_SKU__SEN0257
* @date    7/31/2025
******************************************************************************
*/
 
#include "waterFlow.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "transcoder.h"
#include "stm32wlxx_hal_def.h"
#include "usart.h"

volatile float last_flow_lpm = 0;
volatile unsigned long pulse_count = 0;
const float calibration_factor = 7.5;
SysTime_t currentTime;
SysTime_t lastTime;

HAL_StatusTypeDef FlowInit() { 

    GPIO_InitTypeDef GPIO_InitStruct = {0};
   
    // Configure PA3 as rising edge EXTI input (water flow sensor output)
    __HAL_RCC_GPIOA_CLK_ENABLE();     // Enable  clock for port A
    //__HAL_RCC_GPIOC_CLK_ENABLE();     // Enable GPIOC clock for port C
   // __HAL_RCC_USART2_CLK_ENABLE();    // Enable USART2 clock

    //reset pins
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_RESET);
    //HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_RESET);

    //Configure PIN 10 on Port A (GPIO input)
    GPIO_InitStruct.Pin = GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING; // interrupt on rising edge
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    //Confirgure PIN 1 on Port C (UART input)
    //GPIO_InitStruct.Pin = GPIO_PIN_1;
    //GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;             // Alternate function push-pull
    //GPIO_InitStruct.Pull = GPIO_NOPULL;                 // or GPIO_PULLUP if needed
    //GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    //GPIO_InitStruct.Alternate = 0x04;                   // AF4 = USART2 on PC1
    //HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /* EXTI interrupt init*/
    HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);   //chat says EXTI3_IRQn  but not function for that
    HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

    currentTime = SysTimeGet();
    lastTime = currentTime;

    return ADC_init();
}

waterFlow FlowGetMeasurment() {

    //get time
    currentTime = SysTimeGet();

    //variable for measurement 
    waterFlow measurment;

    if ((currentTime.Seconds != lastTime.Seconds)) {
        uint32_t pulses = pulse_count;
        pulse_count = 0;

        //get liters per minute
        last_flow_lpm = (((float)pulses * 60.0f)/calibration_factor);

        //reset last time
        lastTime.Seconds = currentTime.Seconds;
        return measurment;
    }

    measurment.flow = last_flow_lpm;

    return measurment;
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    if (GPIO_Pin == GPIO_PIN_10) {
      pulse_count++;
    }
}

size_t WatFlow_measure(uint8_t* data) {
    // get timestamp
    SysTime_t ts = SysTimeGet();
    waterFlow measurment = {};

    /// read measurement
    measurment = FlowGetMeasurment();
    double float_flow = measurment.flow;
    const UserConfiguration* cfg = UserConfigGet();

    // encode measurement
    size_t data_len = EncodeWaterFlowMeasurement(
        ts.Seconds, cfg->logger_id, cfg->cell_id, float_flow, data);

    // return number of bytes in serialized measurement
    return data_len;
}
