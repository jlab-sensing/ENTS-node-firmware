#!/usr/bin/env python

# How to use this file

# Uncomment these lines from ADC_read in adc.c
# // char raw[45];
# // sprintf(raw, "Raw: %x %x  Shifted: %i \r\n\r\n",rx_data[0], rx_data[1], reading);
# // HAL_UART_Transmit(&huart2, (const uint8_t *) raw, 36, 19);
# Input 2 voltages into the SPS board and record their values as y1 and y2 in mV below.
# Record their corresponding raw values as x1 and x2.
# You will have to repeat this process to get the current regrssion

# Change the #define SLOPE and B in ads.h to match what linear_regression.py outputs as SLOPE and B
x1 = 61599
y1 = 390
x2 = 55623
y2 = 990

slope = (y2 -y1) / (x2- x1)

b = y2 - (slope * x2)

print("SLOPE: ", round(slope, 3), "B: ", round(b))