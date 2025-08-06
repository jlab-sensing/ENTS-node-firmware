#include "config_server.hpp"

#include <WebServer.h>
#include <ArduinoLog.h>

#include "configuration.hpp"
#include "protobuf_utils.hpp"
#include "soil_power_sensor.pb.h"
#include "transcoder.h"
#include "validation.hpp"


/** Webserver instance on port 80 */
WebServer server(80);

/** @brief Handle the root path of the web server.
 * 
 * This function serves the root path of the web server, displaying the configuration
 * form for the user to fill out.
 */
void handleRoot();

/**
 * @brief Handle save action.
 */
void handleSave();

/**
 * @brief Validate user inputs from the web form.
 *
 * This function checks the inputs provided by the user in the web form
 * and returns a string containing any validation errors.
 *
 * @return A string with validation errors. Emptyry string for no errors.
 */
String validateInputs();


void handleRoot() {
  String html = R"=====(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>ENTS Configuration</title>
  <style>
    body { font-family: Arial, sans-serif; margin: 20px; }
    .group { border: 1px solid #ddd; padding: 15px; margin-bottom: 20px; border-radius: 5px; }
    h2 { margin-top: 0; color: #444; }
    label { display: block; margin: 10px 0 5px; }
    input, select { width: 100%; padding: 8px; margin-bottom: 10px; box-sizing: border-box; }
    button { background-color: #4CAF50; color: white; padding: 10px 15px; border: none; border-radius: 4px; cursor: pointer; }
    button:hover { background-color: #45a049; }
    .checkbox-group { margin: 10px 0; }
    .checkbox-row {
      display: flex;
      align-items: center;
      margin-bottom: 8px;
    }
    .checkbox-row input[type="checkbox"] {
      width: auto;
      margin-right: 8px;
      margin-bottom: 0;
    }
    .checkbox-row label {
      display: inline;
      margin: 0;
    }
    .error {
    color: red;
    margin: 15px 0;
    padding: 10px;
    border: 1px solid red;
    border-radius: 4px;
    background-color: #ffeeee;
    }
    .success {
      color: green;
      margin: 15px 0;
      padding: 10px;
      border: 1px solid green;
      border-radius: 4px;
      background-color: #eeffee;
      }
      #wifiSettings { display: none; }
      .error { color: red; margin-top: 10px; }
      /* Modal styles */
      .modal {
        display: none;
        position: fixed;
        z-index: 1;
        left: 0;
        top: 0;
        width: 100%;
        height: 100%;
        background-color: rgba(0,0,0,0.4);
      }
      
      .modal-content {
        background-color: #fefefe;
        margin: 15% auto;
        padding: 20px;
        border: 1px solid #888;
        width: 80%;
        max-width: 500px;
        border-radius: 5px;
        text-align: center;
      }
      
      .close {
        color: #aaa;
        float: right;
        font-size: 28px;
        font-weight: bold;
        cursor: pointer;
      }
      
      .close:hover {
        color: black;
      }
      </style>
</head>
<body>
  <div id="configModal" class="modal">
    <div class="modal-content">
      <span class="close">&times;</span>
      <h2 id="modalTitle">Configuration Saved</h2>
      <p id="modalMessage"></p>
      <button id="modalButton">OK</button>
    </div>
  </div>
  
  <h1>ENTS Configuration</h1>
 <!-- Error display will appear here, above the form buttons -->
  <div id="errorDisplay" class="error" style="display: none;"></div>
  
  <form id="configForm" novalidate>
    
    <div class="group">
      <h2>Upload Settings</h2>
      <label for="logger_id">Logger ID:</label>
      <input type="text" id="logger_id" name="logger_id" min="0" placeholder="Enter Logger ID (positive integer)">
      
      <label for="cell_id">Cell ID:</label>
      <input type="text" id="cell_id" name="cell_id" min="0" placeholder="Enter Cell ID (positive integer)">
      
      <label for="upload_method">Upload Method:</label>
      <select id="upload_method" name="upload_method" onchange="toggleWifiSettings()">
        <option value="WiFi">WiFi</option>
        <option value="LoRa">LoRa</option>
      </select>
      
      <label for="upload_interval">Upload Interval (seconds):</label>
      <input type="text" id="upload_interval" name="upload_interval" min="1" placeholder="Enter upload interval in seconds">
    </div>
    
    <div class="group">
      <h2>Measurement Settings</h2>
      <div class="checkbox-group">
        <div class="checkbox-row">
          <input type="checkbox" id="voltage_enabled" name="voltage_enabled" checked>
          <label for="voltage_enabled">Voltage</label>
        </div>
        
        <div class="checkbox-row">
          <input type="checkbox" id="current_enabled" name="current_enabled" checked>
          <label for="current_enabled">Current</label>
        </div>
        
        <div class="checkbox-row">
          <input type="checkbox" id="teros12_enabled" name="teros12_enabled" checked>
          <label for="teros12_enabled">Teros12</label>
        </div>
        
        <div class="checkbox-row">
          <input type="checkbox" id="teros21_enabled" name="teros21_enabled" checked>
          <label for="teros21_enabled">Teros21</label>
        </div>
        
        <div class="checkbox-row">
          <input type="checkbox" id="bme280_enabled" name="bme280_enabled" checked>
          <label for="bme280_enabled">BME280</label>
        </div>
      </div>
      
      <label for="calibration_v_slope">Calibration V Slope:</label>
      <input type="text" step="0.0001" id="calibration_v_slope" name="calibration_v_slope" placeholder="Enter Voltage Slope (floating-point)">
      
      <label for="calibration_v_offset">Calibration V Offset:</label>
      <input type="text" step="0.0001" id="calibration_v_offset" name="calibration_v_offset" placeholder="Enter Voltage Offset (floating-point)">
      
      <label for="calibration_i_slope">Calibration I Slope:</label>
      <input type="text" step="0.0001" id="calibration_i_slope" name="calibration_i_slope" placeholder="Enter Current Slope (floating-point)">
      
      <label for="calibration_i_offset">Calibration I Offset:</label>
      <input type="text" step="0.0001" id="calibration_i_offset" name="calibration_i_offset" placeholder="Enter Current Offset (floating-point)">
    </div>
    
    <div class="group" id="wifiSettings">
      <h2>WiFi Settings</h2>
      <label for="wifi_ssid">WiFi SSID:</label>
      <input type="text" id="wifi_ssid" name="wifi_ssid" placeholder="Enter WiFi SSID">
      
      <label for="wifi_password">WiFi Password:</label>
      <input type="password" id="wifi_password" name="wifi_password" placeholder="Enter WiFi Password">
      
      <label for="api_endpoint_url">API Endpoint URL:</label>
      <input type="text" id="api_endpoint_url" name="api_endpoint_url" placeholder="http:// or https://" value="http://dirtviz.jlab.ucsc.edu/api/sensor/">
    </div>
    
    <div id="errorDisplay" class="error"></div>
    <button type="submit">Save Configuration</button>
  </form>
  
  <script>
    // DOM Elements
    const modal = document.getElementById("configModal");
    const modalTitle = document.getElementById("modalTitle");
    const modalMessage = document.getElementById("modalMessage");
    const modalButton = document.getElementById("modalButton");
    const closeBtn = document.getElementsByClassName("close")[0];
    const errorDisplay = document.getElementById('errorDisplay');
    const configForm = document.getElementById("configForm");

    // Show modal with success message
    function showSuccessModal(message) {
      modalTitle.textContent = "Success!";
      modalMessage.textContent = message;
      modal.style.display = "block";
    }

    // Show error message above button
    function showError(message) {
      errorDisplay.textContent = message;
      errorDisplay.style.display = "block";
    }

    // Hide error message
    function hideError() {
      errorDisplay.style.display = "none";
    }

    // Close modal
    function closeModal() {
      modal.style.display = "none";
    }

    // Toggle WiFi settings based on upload method
    function toggleWifiSettings() {
      const method = document.getElementById('upload_method').value;
      const wifiSettings = document.getElementById('wifiSettings');
      wifiSettings.style.display = method === 'WiFi' ? 'block' : 'none';
      
      const wifiFields = wifiSettings.querySelectorAll('input');
      wifiFields.forEach(field => {
        field.required = method === 'WiFi';
      });
    }

    // Handle form submission
    async function handleSubmit(event) {
      event.preventDefault();
      hideError();
      
      try {
        const formData = new FormData(configForm);
        
        const response = await fetch('/save', {
          method: 'POST',
          body: formData
        });
        
        const result = await response.json();
        
        if (!response.ok) {
          throw new Error(result.error || 'Failed to save configuration');
        }
        
        // Show success modal if save was successful
        showSuccessModal(result.success || "Configuration saved successfully!");
        
      } catch (error) {
        showError(error.message);
        console.error('Error:', error);
      }
    }

    // Event listeners
    closeBtn.onclick = closeModal;
    modalButton.onclick = closeModal;
    window.onclick = function(event) {
      if (event.target === modal) {
        closeModal();
      }
    };
    
    document.getElementById('upload_method').addEventListener('change', toggleWifiSettings);
    configForm.addEventListener('submit', handleSubmit);

    // Initialize on page load
    window.onload = function() {
      toggleWifiSettings();
    };
  </script>
</body>
</html>
)=====";

  server.send(200, "text/html", html);
}

void handleSave() {
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

    // copy ssid
    String wifi_ssid = server.arg("wifi_ssid");
    wifi_ssid.trim();
    strncpy(config.WiFi_SSID, wifi_ssid.c_str(),
            sizeof(config.WiFi_SSID));
   
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
  } else {
    Log.errorln("Invalid upload method: %s.", upload_method.c_str());
    Log.errorln("Defaulting to LoRa.");
  }
    
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

  setConfig(config);

  // NOTE: Direclty setting
  // Update the module's current configuration
  //user_config.updateConfig(&decoded_config);

  // Prepare success message
  String successMessage = "Configuration saved successfully!\\n";
  successMessage += "Please RESET the STM32 to update the configurations";
  String successJson = "{\"success\":\"" + successMessage + "\"}";
  server.send(200, "application/json", successJson);
}

void setupServer() {
  server.on("/", HTTP_GET, handleRoot);
  server.on("/save", HTTP_POST, handleSave);
  server.begin();
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
