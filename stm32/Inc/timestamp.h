/**
 * @file timestamp.h
 * @author John Madden (jtmadden@ucsc.edu)
 * @brief Provides quick timestamp functions
 * @date 2024-04-02
 * 
 * This library is designed as a global way to handle timestamping. The typical
 * use case is handling adding accurate timestamps to measurements.
 * 
 * Requires the RTC module to be initialized.
 */

#ifndef __TIMESTAMP_H__
#define __TIMESTAMP_H__

#include "time.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief Gets the current timestamp
 * 
 * The timestamp is represented as unix epochs since January 1st 1970. The
 * timezone of the device is considered.
 * 
 * @return 
 */
time_t TimestampNow(void);

/**
 * @brief Updates the current timestamp from a clock server
 * 
 * A request is sent to a known clock source which returns the "real" timestamp.
 * The RTC is updated to sync with the remote clock source.
 * 
 * @return Timestamp received during sync
 */
time_t TimestampUpdate(void);

#ifdef __cplusplus
}
#endif

#endif /* __TIMESTAMP_H */