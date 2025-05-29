#include "validation.h"
#include "configuration.h"

extern WebServer server;

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

String validateInputs() {
    String error;
    
    // Validate Upload Settings
    if ((error = validateUInt(server.arg("logger_id"), "Logger ID")) != "") return error;
    if ((error = validateUInt(server.arg("cell_id"), "Cell ID")) != "") return error;
    if ((error = validateUInt(server.arg("upload_interval"), "Upload Interval")) != "") return error;
  
    // Validate Measurement Settings
    if ((error = validateFloat(server.arg("calibration_v_slope"), "Calibration V Slope")) != "") return error;
    if ((error = validateFloat(server.arg("calibration_v_offset"), "Calibration V Offset")) != "") return error;
    if ((error = validateFloat(server.arg("calibration_i_slope"), "Calibration I Slope")) != "") return error;
    if ((error = validateFloat(server.arg("calibration_i_offset"), "Calibration I Offset")) != "") return error;
  
    // Validate WiFi Settings if WiFi is selected
    if (server.arg("upload_method") == "WiFi") {
      if (server.arg("wifi_ssid").length() == 0) return "WiFi SSID cannot be empty";
      if ((error = validateURL(server.arg("api_endpoint_url"))) != "") return error;
    }
  
    return ""; // Empty string when no error
  }
