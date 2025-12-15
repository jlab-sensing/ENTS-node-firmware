/**
 ******************************************************************************
 * @file     solenoid.h
 * @author   Caden Jacobs
 * @brief    This file contains all the function prototypes for
 *           the solenoid.c file.
 *
 *           This library is designed to integrate a solenoid used for
 *irrigation.
 *           https://www.orbitonline.com/products/solenoids?variant=45109892612264&country=US&currency=USD&utm_medium=product_sync&utm_source=google&utm_content=sag_organic&utm_campaign=sag_organic&utm_source=paid+search&utm_medium=google+ads&utm_campaign=core+shopping&gad_source=1&gad_campaignid=19652308266&gbraid=0AAAAACw56Spsna6BxGUmTXQEtBiAdD4xG&gclid=CjwKCAjw-svEBhB6EiwAEzSdrsAHw1dSYI86-bRo6kxv42twG7OPn3huLm-9Bx99XAEWvaCfzjyhmxoCmnAQAvD_BwE
 * @date     8/6/2025
 ******************************************************************************
 */

#ifndef LIB_SOLENOID_INCLUDE_SOLENOID_H_
#define LIB_SOLENOID_INCLUDE_SOLENOID_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>

#include "ads.h"

/** States of the solenoid */
typedef enum {
  SOLENOID_OFF,
  SOLENOID_ON,
} StatusSolenoid;

/**
 ******************************************************************************
 * @brief    Wrapper function for the Solenoid initilization.
 *
 * @param    void
 * @return   HAL_StatusTypeDef
 ******************************************************************************
 */
void SolenoidInit(void);

/**
 ******************************************************************************
 * @brief    Returns both the raw voltage value and a calibrated measurment
 *           from a water Flow sensor.
 *
 * @param    void
 * @return   measurments
 ******************************************************************************
 */
void SolenoidOpen(void);

/**
 * @brief Read water Flow sensor and serialize measurement
 *
 * The voltage output of the water Flow is measured. A calibration is applied
 * to convert voltage into a leaf wetness measurement.
 *
 * Current voltage and Flow are the same value, until a calibration
 * is obtained.
 *
 *
 * @see SensorsPrototypeMeasure
 */
void SolenoidClose(void);

#ifdef __cplusplus
}
#endif

#endif  // LIB_CAPSOIL_INCLUDE_CAPSOIL_H_