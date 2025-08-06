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

#define FLOW_AVG_COUNT 5

//Variables
volatile float last_flow_lpm = 0;
volatile unsigned long pulse_count = 0;
const float calibration_factor = 7.5;
SysTime_t currentTime;
SysTime_t lastTime;
float flow_history[FLOW_AVG_COUNT] = {0};
uint8_t flow_index = 0;

HAL_StatusTypeDef FlowInit() { 

    GPIO_InitTypeDef GPIO_InitStruct = {0};
   
    // Configure PA10 as rising edge EXTI input (water flow sensor output)
    __HAL_RCC_GPIOA_CLK_ENABLE();     // Enable  clock for port A

    //reset pins
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_RESET);

    //Configure PIN 10 on Port A (GPIO input)
    GPIO_InitStruct.Pin = GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING; // interrupt on rising edge
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* EXTI interrupt init*/
    HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);  
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

    SysTime_t diff = SysTimeSub(currentTime,lastTime);

    //Sampling time of 0.1s
    if ( diff.SubSeconds >= 100) {
        //__disable_irq();
        uint32_t pulses = pulse_count;
        pulse_count = 0;
       // __enable_irq();

        //Calculate liters per minute
        last_flow_lpm = (((float)pulses * 10.0f)/calibration_factor);

        //Update history
        flow_history[flow_index] = last_flow_lpm;
        flow_index = (flow_index + 1) % FLOW_AVG_COUNT;

        //reset last time
        lastTime.SubSeconds = currentTime.SubSeconds;
    }

    // Calculate average
    float sum = 0.0f;
    for (int i = 0; i < FLOW_AVG_COUNT; i++) {
        sum += flow_history[i];
    }
    measurment.flow = sum / FLOW_AVG_COUNT;

    //measurment.flow = last_flow_lpm;

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
