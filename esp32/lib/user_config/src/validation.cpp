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
  for (unsigned int i = 0; i < value.length(); i++) {
    if (i == 0 && value.charAt(i) == '-') continue;
    if (value.charAt(i) == '.' && !decimal_point) {
      decimal_point = true;
      continue;
    }
    if (!isdigit(value.charAt(i))) {
      return name + " must be a valid number";
    }
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

