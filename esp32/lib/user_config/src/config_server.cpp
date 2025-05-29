#include "config_server.h"
#include "configuration.h"
#include "validation.h"
#include "protobuf_utils.h"
#include "soil_power_sensor.pb.h"
#include "transcoder.h"

WebServer server(80);
const char* ap_ssid = "ESP32-C3-Config";
const char* ap_password = "configureme";

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
  </style>
</head>
<body>
  <h1>ENTS Configuration</h1>
  <form action="/save" method="post" onsubmit="return validateForm()" novalidate>
    
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
    function toggleWifiSettings() {
      const method = document.getElementById('upload_method').value;
      const wifiSettings = document.getElementById('wifiSettings');
      wifiSettings.style.display = method === 'WiFi' ? 'block' : 'none';
      
      // Toggle required attribute for WiFi fields
      const wifiFields = wifiSettings.querySelectorAll('input');
      wifiFields.forEach(field => {
        field.required = method === 'WiFi';
      });
    }
    
function validateForm() {
  const form = document.querySelector('form');
  const errorDisplay = document.getElementById('errorDisplay');
  errorDisplay.textContent = '';
  errorDisplay.className = 'error'; // Reset to error style
  
  event.preventDefault();
  
  const formData = new FormData(form);
  
  fetch('/save', {
    method: 'POST',
    headers: {
      'Accept': 'application/json'
    },
    body: formData
  })
  .then(response => {
    if (!response.ok) {
      return response.json().then(err => { 
        throw new Error(err.error || 'Unknown error'); 
      });
    }
    return response.json();
  })
  .then(data => {
    if (data.success) {
      errorDisplay.textContent = data.success;
      errorDisplay.className = 'success';
    }
  })
  .catch(error => {
    errorDisplay.textContent = error.message;
    errorDisplay.className = 'error';
    console.error('Error:', error); // Debug output
  });
}
    
    // Initialize on page load
    window.onload = toggleWifiSettings;
  </script>
</body>
</html>
)=====";

  server.send(200, "text/html", html);
}

void handleSave() {
    String error = validateInputs();
    if (error != "") {
      error.replace("\"", "\\\"");
      String errorJson = "{\"error\":\"" + error + "\"}";
      server.send(400, "application/json", errorJson);
      return;
    }
  
    // Parsing the webpage data into our config structure
    config.logger_id = server.arg("logger_id").toInt();
    config.cell_id = server.arg("cell_id").toInt();
    config.upload_method = server.arg("upload_method");
    config.upload_interval = server.arg("upload_interval").toInt();
    
    config.voltage_enabled = server.hasArg("voltage_enabled");
    config.current_enabled = server.hasArg("current_enabled");
    config.teros12_enabled = server.hasArg("teros12_enabled");
    config.teros21_enabled = server.hasArg("teros21_enabled");
    config.bme280_enabled = server.hasArg("bme280_enabled");
    
    config.calibration_v_slope = server.arg("calibration_v_slope").toDouble();
    config.calibration_v_offset = server.arg("calibration_v_offset").toDouble();
    config.calibration_i_slope = server.arg("calibration_i_slope").toDouble();
    config.calibration_i_offset = server.arg("calibration_i_offset").toDouble();
    
    if (config.upload_method == "WiFi") {
      config.wifi_ssid = server.arg("wifi_ssid");
      config.wifi_password = server.arg("wifi_password");
      config.api_endpoint_url = server.arg("api_endpoint_url");
      config.api_endpoint_port = 2;
    } else {
      config.wifi_ssid = "";
      config.wifi_password = "";
      config.api_endpoint_url = "";
      config.api_endpoint_port = 0;
    }
  
    printConfiguration();
  
    UserConfiguration pb_config = UserConfiguration_init_zero;
    pb_config.logger_id = config.logger_id;
    pb_config.cell_id = config.cell_id;
    pb_config.Upload_interval = config.upload_interval;
    pb_config.Upload_method = (config.upload_method == "WiFi") ? Uploadmethod_WiFi : Uploadmethod_LoRa;
    
    if (config.voltage_enabled) {
      pb_config.enabled_sensors[pb_config.enabled_sensors_count++] = EnabledSensor_Voltage;
    }
    if (config.current_enabled) {
      pb_config.enabled_sensors[pb_config.enabled_sensors_count++] = EnabledSensor_Current;
    }
    if (config.teros12_enabled) {
      pb_config.enabled_sensors[pb_config.enabled_sensors_count++] = EnabledSensor_Teros12;
    }
    if (config.teros21_enabled) {
      pb_config.enabled_sensors[pb_config.enabled_sensors_count++] = EnabledSensor_Teros21;
    }
    if (config.bme280_enabled) {
      pb_config.enabled_sensors[pb_config.enabled_sensors_count++] = EnabledSensor_BME280;
    }
    
    pb_config.Voltage_Slope = config.calibration_v_slope;
    pb_config.Voltage_Offset = config.calibration_v_offset;
    pb_config.Current_Slope = config.calibration_i_slope;
    pb_config.Current_Offset = config.calibration_i_offset;
    
    if (config.upload_method == "WiFi") {
      strncpy(pb_config.WiFi_SSID, config.wifi_ssid.c_str(), sizeof(pb_config.WiFi_SSID));
      strncpy(pb_config.WiFi_Password, config.wifi_password.c_str(), sizeof(pb_config.WiFi_Password));
      strncpy(pb_config.API_Endpoint_URL, config.api_endpoint_url.c_str(), sizeof(pb_config.API_Endpoint_URL));
      pb_config.API_Endpoint_Port = config.api_endpoint_port;
    }
  
    uint8_t buffer[UserConfiguration_size];
    size_t message_length = EncodeUserConfiguration(&pb_config, buffer);
    
    if (message_length > 0) {
      printEncodedData(buffer, message_length);
      
      UserConfiguration decoded_config = UserConfiguration_init_zero;
      if (DecodeUserConfiguration(buffer, message_length, &decoded_config) == 0) {
        printDecodedConfig(&decoded_config);
      } else {
        Serial.println("Failed to decode the configuration");
      }
    } else {
      Serial.println("Failed to encode the configuration");
    }
    String successResponse = "{\"success\":\"Configuration saved successfully!\"}";
    server.send(200, "application/json", successResponse);
}

void setupServer() {
  server.on("/", HTTP_GET, handleRoot);
  server.on("/save", HTTP_POST, handleSave);
  server.begin();
}

void handleClient() {
  server.handleClient();
}