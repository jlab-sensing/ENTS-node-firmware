#include "modules/wifi.hpp"

#include <ArduinoLog.h>

#include "http.hpp"

ModuleWiFi::ModuleWiFi(void) {
  // set module type
  type = Esp32Command_wifi_command_tag;

  // create ntp client
  timeClient = new NTPClient(ntpUDP);
}

ModuleWiFi::~ModuleWiFi(void) {
  // delete timeClient
  delete timeClient;
}

void ModuleWiFi::OnReceive(const Esp32Command &cmd) {
  Log.traceln("ModuleWiFi::OnReceive");

  // check if WiFi command
  if (cmd.which_command != Esp32Command_wifi_command_tag) {
    return;
  }

  Log.traceln("WiFiCommand type: %d", cmd.command.wifi_command.type);

  // switch for command types
  switch (cmd.command.wifi_command.type) {
    case WiFiCommand_Type_CONNECT:
      Log.traceln("Calling CONNECT");
      Connect(cmd);
      break;

    case WiFiCommand_Type_POST:
      Log.traceln("Calling POST");
      Post(cmd);
      break;

    case WiFiCommand_Type_CHECK:
      Log.traceln("Calling CHECK");
      Check(cmd);
      break;

    case WiFiCommand_Type_TIME:
      Log.traceln("Calling TIME");
      Time(cmd);
      break;

    default:
      Log.warningln("wifi command type not found!");
      break;
  }
}

size_t ModuleWiFi::OnRequest(uint8_t *buffer) {
  Log.traceln("ModuleWiFi::OnRequest");
  memcpy(buffer, request_buffer, request_buffer_len);
  return request_buffer_len;
}

void ModuleWiFi::Connect(const Esp32Command &cmd) {
  // init return command
  WiFiCommand wifi_cmd = WiFiCommand_init_zero;
  wifi_cmd.type = WiFiCommand_Type_CONNECT;

  Log.traceln("ModuleWiFi::Connect");

  //  print the mac address
  uint8_t mac[6];
  WiFi.macAddress(mac);
  Log.noticeln("MAC: %x:%x:%x:%x:%x:%x", mac[0], mac[1], mac[2], mac[3], mac[4],
               mac[5]);

  Log.noticeln("Connecting to WiFI...");
  Log.noticeln("ssid: %s", cmd.command.wifi_command.ssid);
  Log.noticeln("passwd: %s", cmd.command.wifi_command.passwd);

  // TODO(jmadden173) update hostname to something sane
  // WiFi.setHostname("esp32");

  // connect to WiFi
  WiFi.disconnect();
  int status = WiFi.begin(cmd.command.wifi_command.ssid,
                          cmd.command.wifi_command.passwd);

  Log.noticeln("WiFi connection status: %d", status);

  // set status
  wifi_cmd.rc = status;

  request_buffer_len =
      EncodeWiFiCommand(&wifi_cmd, request_buffer, sizeof(request_buffer));
}

void ModuleWiFi::Post(const Esp32Command &cmd) {
  // init return WiFi command
  WiFiCommand wifi_cmd = WiFiCommand_init_zero;
  wifi_cmd.type = WiFiCommand_Type_POST;

  Log.traceln("ModuleWiFI::Post");

  // check if connected to WiFi connected
  if (WiFi.status() != WL_CONNECTED) {
    Log.errorln("Not connected to WiFi!");

    wifi_cmd.rc = 1;
  } else {
    // send measurement
    const uint8_t *meas = cmd.command.wifi_command.resp.bytes;
    const size_t meas_len = cmd.command.wifi_command.resp.size;

    // send measurement
    HttpClient resp_msg = this->dirtviz.SendMeasurement(meas, meas_len);
    const uint8_t *resp =
        reinterpret_cast<const uint8_t *>(resp_msg.Data().c_str());
    size_t resp_len = resp_msg.Data().length();
    unsigned int status_code = resp_msg.ResponseCode();

    Log.noticeln("Resp length: %d", resp_len);

    // save status code and response
    wifi_cmd.rc = status_code;
    if (resp_len > 0) {
      wifi_cmd.resp.size = resp_len;
      memcpy(wifi_cmd.resp.bytes, resp, resp_len);
    }
  }

  // encode wifi command in buffer
  this->request_buffer_len =
      EncodeWiFiCommand(&wifi_cmd, request_buffer, sizeof(request_buffer));
}

void ModuleWiFi::Check(const Esp32Command &cmd) {
  Log.traceln("ModuleWiFi::Check");

  WiFiCommand wifi_cmd = WiFiCommand_init_zero;
  wifi_cmd.type = WiFiCommand_Type_CHECK;

  // set url and port
  dirtviz.SetUrl(cmd.command.wifi_command.url);

  int status = WiFi.status();
  wifi_cmd.rc = status;
  if (status == WL_CONNECTED) {
    Log.noticeln("IP Address: %p", WiFi.localIP());
    Log.noticeln("Gateway IP: %p", WiFi.gatewayIP());
    Log.noticeln("Subnet Mask: %p", WiFi.subnetMask());
    Log.noticeln("DNS: %p", WiFi.dnsIP());

    Log.noticeln("Checking API endpoint");
    wifi_cmd.rc = dirtviz.Check();
    Log.noticeln("Response code: %d", wifi_cmd.rc);

  } else if (status == WL_CONNECT_FAILED) {
    Log.errorln("Connection failed!");
  } else if (status == WL_NO_SSID_AVAIL) {
    Log.errorln("SSID not available!");
  }

  request_buffer_len =
      EncodeWiFiCommand(&wifi_cmd, request_buffer, sizeof(request_buffer));
}

void ModuleWiFi::Time(const Esp32Command &cmd) {
  Log.traceln("ModuleWiFi::Time");

  WiFiCommand wifi_cmd = WiFiCommand_init_zero;
  wifi_cmd.type = WiFiCommand_Type_TIME;

  // check if connected to WiFi connected
  int wifi_status = WiFi.status();
  wifi_cmd.rc = wifi_status;
  if (wifi_status != WL_CONNECTED) {
    Log.errorln("Not connected to WiFi!");
  } else {
    // start timeclient
    timeClient->begin();
    // force update
    if (timeClient->update()) {
      wifi_cmd.ts = timeClient->getEpochTime();
      Log.noticeln("Current timestamp: %d", wifi_cmd.ts);
    } else {
      Log.errorln("Failed to get time from NTP server!");
    }
  }

  request_buffer_len =
      EncodeWiFiCommand(&wifi_cmd, request_buffer, sizeof(request_buffer));
}
