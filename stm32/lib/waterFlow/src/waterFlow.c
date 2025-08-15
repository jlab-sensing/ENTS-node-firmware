/**
 ******************************************************************************
* @file    waterFlow.c
* @author  Caden Jacobs
*
* @brief   This library is designed to read measurements from a Water Flow Sensor
*          https://www.danomsk.ru/upload/iblock/43d/193917_3b664efb7b37f7ae8ea1eea40978a265.pdf
*          
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
static volatile float last_flow_lpm = 0;
static volatile unsigned long pulse_count = 0;
SysTime_t currentTime;
SysTime_t lastTime;
float flow_history[FLOW_AVG_COUNT] = {0};
uint8_t flow_index = 0;

//For every one liter of water that passes through the sensor in one minute, there are 450 pulses. 
//Therefore the calibration factor becomes [450/60 = 7.5]
const float calibration_factor = 7.5;

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

    //Get INIT Times
    currentTime = SysTimeGet();
    lastTime = currentTime;
}

YFS210CMeasurement FlowGetMeasurment() {

    //get time
    currentTime = SysTimeGet();

    //variable for measurement 
    YFS210CMeasurement flowMeas;

    SysTime_t diff = SysTimeSub(currentTime,lastTime);

    //Sampling time of 0.1s
    if ( diff.SubSeconds >= 100) {

        uint32_t pulses = pulse_count;
        pulse_count = 0;

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
    flowMeas.flow = sum / FLOW_AVG_COUNT;

    return flowMeas;
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    if (GPIO_Pin == GPIO_PIN_10) {
      pulse_count++;
    }
}

size_t WatFlow_measure(uint8_t* data) {
    // get timestamp
    SysTime_t ts = SysTimeGet();
    YFS210CMeasurement flowMeas = {};

    /// read measurement
    flowMeas = FlowGetMeasurment();
    const UserConfiguration* cfg = UserConfigGet();

    // encode measurement
    size_t data_len = EncodeWaterFlowMeasurement(
        ts.Seconds, cfg->logger_id, cfg->cell_id, flowMeas.flow, data);

    // return number of bytes in serialized measurement
    return data_len;
}
