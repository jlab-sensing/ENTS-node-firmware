#ifndef LIB_USER_CONFIG_INCLUDE_VALIDATION_HPP
#define LIB_USER_CONFIG_INCLUDE_VALIDATION_HPP

#include <Arduino.h>

/**
 * @brief Validate a string as an unsigned integer.
 *
 * @param value The string value to validate.
 * @param name The name of the value for error reporting.
 *
 * @return An empty string if valid, or an error message if invalid.
 */
String validateUInt(const String& value, const String& name);

/**
 * @brief Validate a string as a floating-point number.
 *
 * @param value The string value to validate.
 * @param name The name of the value for error reporting.
 *
 * @return An empty string if valid, or an error message if invalid.
 */
String validateFloat(const String& value, const String& name);

/**
 * @brief Validate a string as a URL.
 *
 * @param value The string value to validate.
 * @param name The name of the value for error reporting.
 *
 * @return An empty string if valid, or an error message if invalid.
 */
String validateURL(const String& value);

#endif  // LIB_USER_CONFIG_INCLUDE_VALIDATION_HPP
