#include "modules/irrigation.hpp"
#include <ArduinoLog.h>
#include "webserver.hpp"
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <sys/time.h>  // For gettimeofday()
#include <time.h>      // For gmtime() and strftime()

// Soil moisture thresholds
static float moisture_min_threshold = 50.0f;
static float moisture_max_threshold = 75.0f;
static unsigned long check_interval = 10000; // 10 seconds
static unsigned long last_check_time = 0;
bool auto_irrigation_enabled = false;

// Current moisture reading
static float current_moisture_value = -1.0f;

ModuleIrrigation::ModuleIrrigation(void) {
  type = Esp32Command_irrigation_command_tag;
}

ModuleIrrigation::~ModuleIrrigation(void) {
  // nothing to do
}

void ModuleIrrigation::OnReceive(const Esp32Command &cmd) {
  if (cmd.which_command != Esp32Command_irrigation_command_tag) {
    return;
  }

  switch (cmd.command.irrigation_command.type) {
    case IrrigationCommand_Type_CHECK:
      Check(cmd);
      break;
    default:
      break;
  }
}

size_t ModuleIrrigation::OnRequest(uint8_t *buffer) {
  memcpy(buffer, request_buffer, request_buffer_len);
  return request_buffer_len;
}

void ModuleIrrigation::CheckAutoIrrigation() {
  if (!auto_irrigation_enabled) {
    return;
  }

  unsigned long current_time = millis();
  if ((current_time - last_check_time) < check_interval) {
    return;
  }
  
  last_check_time = current_time;
  
  Log.noticeln("Fetching soil moisture from API...");
  float new_moisture = GetSimpleSoilMoisture();
  
  if (new_moisture >= 0) {
    current_moisture_value = new_moisture;
    Log.noticeln("Moisture reading: %.1f%%", new_moisture);
    CheckIrrigationConditions();
  } else {
    Log.errorln("Failed to get moisture from API");
    current_moisture_value = -1.0f;
  }
}

void ModuleIrrigation::Check(const Esp32Command &cmd) {
  IrrigationCommand resp = IrrigationCommand_init_zero;
  resp.state = GetSolenoidState();
  request_buffer_len = EncodeIrrigationCommand(&resp, request_buffer, sizeof(request_buffer));
}

void CheckIrrigationConditions() {
  if (!auto_irrigation_enabled || current_moisture_value < 0) {
    return;
  }
  
  IrrigationCommand_State current_state = GetSolenoidState();
  
  if (current_moisture_value < moisture_min_threshold) {
    Log.noticeln("Moisture low (%.1f%% < %.1f%%) - Opening valve", 
                 current_moisture_value, moisture_min_threshold);
    SetSolenoidState(IrrigationCommand_State_OPEN);
  }
  else if (current_moisture_value > moisture_max_threshold) {
    Log.noticeln("Moisture adequate (%.1f%% > %.1f%%) - Closing valve", 
                 current_moisture_value, moisture_max_threshold);
    SetSolenoidState(IrrigationCommand_State_CLOSE);
  }
}

// Helper function to format time as RFC 1123
String formatTimeRFC1123(time_t timestamp) {
  struct tm *tm_info = gmtime(&timestamp);
  char buf[30];
  strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S GMT", tm_info);
  return String(buf);
}

// Simple API call with proper time formatting
float GetSimpleSoilMoisture() {
  HTTPClient http;
  float moisture_value = -1.0f;

  // Get current UTC time using gettimeofday
  struct timeval tv;
  if (gettimeofday(&tv, NULL) == 0) {
    time_t now = tv.tv_sec;
    time_t start_time = now - 30; // 30 seconds ago
    
    // Format timestamps in RFC 1123 format
    String start_time_str = formatTimeRFC1123(start_time);
    String end_time_str = formatTimeRFC1123(now);
    
    String url = "http://dirtviz.jlab.ucsc.edu/api/sensor/";
    url += "?name=sen0308";
    url += "&measurement=humidity";
    url += "&cellId=1448";
    url += "&startTime=" + start_time_str;
    url += "&endTime=" + end_time_str;
    url += "&stream=true";

    Log.traceln("API URL: %s", url.c_str());
    
    if (http.begin(url)) {
      Log.traceln("Requesting data from last 30 seconds...");
      int httpCode = http.GET();
      Log.traceln("HTTP response code: %d", httpCode);
      
      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        Log.traceln("Payload: %s", payload.c_str());
        
        DynamicJsonDocument doc(2048);
        DeserializationError error = deserializeJson(doc, payload);

        if (!error) {
          // The response is a JSON object with a "data" array
          if (doc.containsKey("data") && doc["data"].is<JsonArray>()) {
            JsonArray data_array = doc["data"];
            
            if (data_array.size() > 0) {
              // Get the most recent reading (last in array)
              moisture_value = data_array[data_array.size() - 1].as<float>();
            } else {
              Log.errorln("Data array is empty");
            }
          } else {
            Log.errorln("No 'data' array in API response");
            // Debug: print all keys and their types
            for (JsonPair kv : doc.as<JsonObject>()) {
              if (kv.value().is<JsonArray>()) {
                Log.traceln("Key: %s (Array)", kv.key().c_str());
              } else {
                Log.traceln("Key: %s, Value: %s", kv.key().c_str(), kv.value().as<String>().c_str());
              }
            }
          }
        } else {
          Log.errorln("JSON parse error: %s", error.c_str());
        }
      } else {
        Log.errorln("HTTP request failed: %d", httpCode);
      }
      http.end();
    } else {
      Log.errorln("Failed to begin HTTP connection");
    }
  } else {
    Log.errorln("Failed to get current time");
  }

  return moisture_value;
}

// Auto irrigation control functions
void EnableAutoIrrigation(float min_thresh, float max_thresh) {
  auto_irrigation_enabled = true;
  moisture_min_threshold = min_thresh;
  moisture_max_threshold = max_thresh;
  Log.noticeln("Auto irrigation enabled: Min=%.1f%%, Max=%.1f%%", min_thresh, max_thresh);
}

void DisableAutoIrrigation() {
  auto_irrigation_enabled = false;
  Log.noticeln("Auto irrigation disabled");
}

bool IsAutoIrrigationEnabled() {
  return auto_irrigation_enabled;
}

float GetMinThreshold() {
  return moisture_min_threshold;
}

float GetMaxThreshold() {
  return moisture_max_threshold;
}

unsigned long GetCheckInterval() {
  return check_interval;
}

void SetCheckInterval(unsigned long interval_ms) {
  check_interval = interval_ms;
}

float GetCurrentMoistureFromCache() {
  return current_moisture_value;
}