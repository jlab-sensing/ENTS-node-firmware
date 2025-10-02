#include "validation.hpp"

String validateUInt(const String& value, const String& name) {
  if (value.length() == 0) {
    return name + " cannot be empty";
  }
  for (unsigned int i = 0; i < value.length(); i++) {
    if (!isdigit(value.charAt(i))) {
      return name + " must be a positive integer";
    }
  }
  return "";
}

String validateFloat(const String& value, const String& name) {
  if (value.length() == 0) {
    return name + " cannot be empty";
  }

  bool decimal_point = false;
  bool exponent = false;
  bool digit_seen = false;
  bool digit_after_exp = true;

  for (unsigned int i = 0; i < value.length(); i++) {
    char c = value.charAt(i);

    // Allow leading sign
    if (i == 0 && (c == '-' || c == '+')) continue;

    // Handle digits
    if (isdigit(c)) {
      digit_seen = true;
      if (exponent) digit_after_exp = true;
      continue;
    }

    // Handle decimal point
    if (c == '.' && !decimal_point && !exponent) {
      decimal_point = true;
      continue;
    }

    // Handle exponent (e/E)
    if ((c == 'e' || c == 'E') && !exponent && digit_seen) {
      exponent = true;
      digit_after_exp = false;

      // Allow sign right after exponent
      if (i + 1 < value.length() && (value.charAt(i+1) == '+' || value.charAt(i+1) == '-')) {
        i++;
      }
      continue;
    }

    // If we get here, invalid character
    return name + " must be a valid number";
  }

  // Must have at least one digit, and if exponent present, digits after it
  if (!digit_seen || !digit_after_exp) {
    return name + " must be a valid number";
  }

  return "";
}

String validateURL(const String& value) {
  if (value.length() == 0) {
    return "API Endpoint URL cannot be empty";
  }
  if (!value.startsWith("http://") && !value.startsWith("https://")) {
    return "API Endpoint URL must start with http:// or https://";
  }
  return "";
}
