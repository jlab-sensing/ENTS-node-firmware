#include "config_server.hpp"

#include <ArduinoLog.h>
#include <LittleFS.h>
#include <WebServer.h>

#include "configuration.hpp"
#include "protobuf_utils.hpp"
#include "soil_power_sensor.pb.h"
#include "transcoder.h"
#include "validation.hpp"

/** Webserver instance on port 80 */
WebServer server(80);

/** @brief Handle the root path of the web server.
 *
 * This function serves the root path of the web server, displaying the
 * configuration form for the user to fill out.
 */
void handleRoot();

/**
 * @brief Handle save action.
 */
void handleSave();

/**
 * @brief Get the current configuration
 *
 * Returns the user config as a json object
 */
void handleConfig();

/**
 * @brief Prints debug information for http queries.
 *
 * Must be called within the web server request handler to log the query.
 */
void printQuery();

/**
 * @brief Validate user inputs from the web form.
 *
 * This function checks the inputs provided by the user in the web form
 * and returns a string containing any validation errors.
 *
 * @return A string with validation errors. Emptyry string for no errors.
 */
String validateInputs();

void handleSave() {
  printQuery();

  // validate inputs and return error if any
  String error = validateInputs();
  if (error != "") {
    error.replace("\"", "\\\"");
    String errorJson = "{\"error\":\"" + error + "\"}";
    server.send(400, "application/json", errorJson);
    return;
  }

  UserConfiguration config = UserConfiguration_init_default;

  // Parsing the webpage data into our config structure
  config.logger_id = server.arg("logger_id").toInt();
  config.cell_id = server.arg("cell_id").toInt();

  // set upload method
  String upload_method = server.arg("upload_method");
  if (upload_method == "LoRa") {
    config.Upload_method = Uploadmethod_LoRa;
  } else if (upload_method == "WiFi") {
    config.Upload_method = Uploadmethod_WiFi;
  } else {
    Log.errorln("Invalid upload method: %s.", upload_method.c_str());
    Log.errorln("Defaulting to LoRa.");
    config.Upload_method = Uploadmethod_LoRa;
  }

  // copy ssid
  String wifi_ssid = server.arg("wifi_ssid");
  wifi_ssid.trim();
  strncpy(config.WiFi_SSID, wifi_ssid.c_str(), sizeof(config.WiFi_SSID));

  // copy password
  String wifi_password = server.arg("wifi_password");
  wifi_password.trim();
  strncpy(config.WiFi_Password, wifi_password.c_str(),
          sizeof(config.WiFi_Password));

  // copy url
  String api_endpoint_url = server.arg("api_endpoint_url");
  api_endpoint_url.trim();
  strncpy(config.API_Endpoint_URL, api_endpoint_url.c_str(),
          sizeof(config.API_Endpoint_URL));

  config.Upload_interval = server.arg("upload_interval").toInt();

  bool voltage_enabled = server.hasArg("voltage_enabled");
  if (voltage_enabled) {
    config.enabled_sensors[config.enabled_sensors_count++] =
        EnabledSensor_Voltage;
  }

  bool current_enabled = server.hasArg("current_enabled");
  if (current_enabled) {
    config.enabled_sensors[config.enabled_sensors_count++] =
        EnabledSensor_Current;
  }

  bool teros12_enabled = server.hasArg("teros12_enabled");
  if (teros12_enabled) {
    config.enabled_sensors[config.enabled_sensors_count++] =
        EnabledSensor_Teros12;
  }

  bool teros21_enabled = server.hasArg("teros21_enabled");
  if (teros21_enabled) {
    config.enabled_sensors[config.enabled_sensors_count++] =
        EnabledSensor_Teros21;
  }

  bool bme280_enabled = server.hasArg("bme280_enabled");
  if (bme280_enabled) {
    config.enabled_sensors[config.enabled_sensors_count++] =
        EnabledSensor_BME280;
  }

  config.Voltage_Slope = server.arg("calibration_v_slope").toDouble();
  config.Voltage_Offset = server.arg("calibration_v_offset").toDouble();
  config.Current_Slope = server.arg("calibration_i_slope").toDouble();
  config.Current_Offset = server.arg("calibration_i_offset").toDouble();

  // POSTED config
  // Log.noticeln("Received configuration:");
  // Log.noticeln("-----------------------");
  // printConfig(config);

  setConfig(config);
  printReceivedConfig();

  // NOTE: Direclty setting
  // Update the module's current configuration
  // user_config.updateConfig(&decoded_config);

  // Prepare success message
  String successMessage = "Configuration saved successfully!\\n";
  successMessage += "Please RESET the STM32 to update the configurations";
  String successJson = "{\"success\":\"" + successMessage + "\"}";
  server.send(200, "application/json", successJson);
}

void handleConfig() {
  printQuery();

  String json = getConfigJson();

  server.send(200, "application/json", json);
}

void setupServer() {
  // start server once
  bool started = false;
  if (!started) {
    server.serveStatic("/", LittleFS, "/index.html");
    server.on("/save", HTTP_POST, handleSave);
    server.on("/config", HTTP_GET, handleConfig);
    server.begin();

    started = true;
  }
}

void handleClient() { server.handleClient(); }

String validateInputs() {
  String error;

  // Validate Upload Settings
  if ((error = validateUInt(server.arg("logger_id"), "Logger ID")) != "")
    return error;
  if ((error = validateUInt(server.arg("cell_id"), "Cell ID")) != "")
    return error;
  if ((error = validateUInt(server.arg("upload_interval"),
                            "Upload Interval")) != "")
    return error;

  // Validate Measurement Settings
  if ((error = validateFloat(server.arg("calibration_v_slope"),
                             "Calibration V Slope")) != "")
    return error;
  if ((error = validateFloat(server.arg("calibration_v_offset"),
                             "Calibration V Offset")) != "")
    return error;
  if ((error = validateFloat(server.arg("calibration_i_slope"),
                             "Calibration I Slope")) != "")
    return error;
  if ((error = validateFloat(server.arg("calibration_i_offset"),
                             "Calibration I Offset")) != "")
    return error;

  // Validate WiFi Settings if WiFi is selected
  if (server.arg("upload_method") == "WiFi") {
    if (server.arg("wifi_ssid").length() == 0)
      return "WiFi SSID cannot be empty";
    if ((error = validateURL(server.arg("api_endpoint_url"))) != "")
      return error;
  }

  return "";  // Empty string when no error
}

void printQuery() {
  IPAddress ip = server.client().remoteIP();
  String path = server.uri();

  std::string method = "";
  switch (server.method()) {
    case HTTP_GET:
      method = "GET";
      break;
    case HTTP_POST:
      method = "POST";
      break;
    case HTTP_PUT:
      method = "PUT";
      break;
    case HTTP_DELETE:
      method = "DELETE";
      break;
    default:
      method = "UNKNOWN";
  }

  Log.noticeln("%s - %s %s", ip.toString().c_str(), method.c_str(),
               path.c_str());
}
