/**
 * @brief Library used to interface with the Dirtviz API
 *
 * @author John Madden <jmadden173@pm.me>
 * @date 2023-11-29
 */

/**
 * @ingroup esp32
 * @defgroup dirtviz Dirtviz
 * @brief Library used to interface with the Dirtviz API
 */

#ifndef LIB_DIRTVIZ_INCLUDE_DIRTVIZ_HPP_
#define LIB_DIRTVIZ_INCLUDE_DIRTVIZ_HPP_

#include <LCBUrl.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "http.hpp"

/**
 * @ingroup dirtviz
 * @brief Logic for interfacing with the Dirtviz API
 *
 * Assumes that WiFi interface is connected to the network. Uses @ref http to
 * read http requests. Requests are formatted ad-hoc.
 *
 * Examples:
 * - @ref example_dirtviz.cpp
 *
 * @{
 */

/**
 * @brief HTTP interface for Dirtviz API
 */
class Dirtviz {
 public:
  /**
   * @brief Construct a new Dirtviz object
   *
   */
  Dirtviz(void);

  /**
   * @brief Default constructor
   *
   * Allows for setting of URL on initialization.
   *
   * @param url API URL
   */
  explicit Dirtviz(const char *url);

  /**
   * @brief Frees all memory
   */
  ~Dirtviz();

  /**
   * @brief Set new URL string
   *
   * @param new_url New API URL
   */
  void SetUrl(const char *url);

  /**
   * @brief Health check for API endpoint
   *
   * @return HTTP code from health check
   */
  unsigned int Check();

  /**
   * @brief Send serialized measurement to the API
   *
   * @param meas Pointer to serialized measurement data
   * @param meas_len Number of bytes in @p meas
   *
   * @return Response from http server
   */
  HttpClient SendMeasurement(const uint8_t *meas, size_t meas_len);

 private:
  /** URL of API */
  LCBUrl url;

  /**
   * @brief Starts connection with server
   *
   * @return Returns true if the connection succeeds, false if not
   */
  bool ClientConnect();
};

/**
 * @}
 */

#endif  // LIB_DIRTVIZ_INCLUDE_DIRTVIZ_HPP_
