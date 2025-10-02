/**
 * @file user_config.h
 * @author John Madden
 * @brief Controls the user config webserver
 * @date 2025-10-01
 *
 * This files interfaces with the user config webserver. It allows starting and
 * stopping of the webserver. There is an optional timeout to stop the server
 * after a period of inactivity (no clients connected).
 */

#ifndef INC_USER_CONFIG_H_
#define INC_USER_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Starts the user config webserver
 */
void UserConfigStart();

/**
 * @brief Stops the user config webserver after timeout.
 *
 * Must be called after time synchronization has been established.
 *
 * @param timeout Timeout in seconds to stop the server after inactivity.
 */
void UserConfigSetupStop(unsigned int timeout);

/**
 * @brief Stops the user config webserver
 */
void UserConfigStop(void);

#ifdef __cplusplus
}
#endif

#endif  // INC_USER_CONFIG_H_
