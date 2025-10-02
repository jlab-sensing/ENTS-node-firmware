#ifndef LIB_USER_CONFIG_INCLUDE_CONFIG_SERVER_H
#define LIB_USER_CONFIG_INCLUDE_CONFIG_SERVER_H

/**
 * @brief Setup webserver
 */
void setupServer();

/**
 * @brief Call to handle requests to the web server.
 *
 * This function should be called in the main loop to process incoming HTTP
 * requests.
 */
void handleClient();

#endif  // LIB_USER_CONFIG_INCLUDE_CONFIG_SERVER_H
