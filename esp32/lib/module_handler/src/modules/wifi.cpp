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
      Log.traceln("Calling CHECK_REQUEST");
      CheckRequest(cmd);
      break;

    case WiFiCommand_Type_TIME:
      Log.traceln("Calling TIME");
      Time(cmd);
      break;

    case WiFiCommand_Type_DISCONNECT:
      Log.traceln("Calling DISCONNECT");
      Disconnect(cmd);
      break;

    case WiFiCommand_Type_CHECK_WIFI:
      Log.traceln("Calling CHECK_WIFI");
      CheckWiFi(cmd);
      break;

    case WiFiCommand_Type_CHECK_API:
      Log.traceln("Calling CHECK_API");
      CheckApi(cmd);
      break;

    case WiFiCommand_Type_NTP_SYNC:
      Log.traceln("Calling NTP_SYNC");
      NtpSync(cmd);
      break;

    case WiFiCommand_Type_HOST:
      Log.traceln("Calling HOST");
      Host(cmd);
      break;

    case WiFiCommand_Type_STOP_HOST:
      Log.traceln("Calling STOP_HOST");
      StopHost();
      break;

    case WiFiCommand_Type_HOST_INFO:
      Log.traceln("Calling HOST_INFO");
      HostInfo();
      break;

    default:
      Log.warningln("wifi command type %d not found!", cmd.command.wifi_command.type);
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
  // WiFi.disconnect();
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

  // send measurement
  const uint8_t *meas = cmd.command.wifi_command.resp.bytes;
  const size_t meas_len = cmd.command.wifi_command.resp.size;
  dirtviz.SendMeasurement(meas, meas_len);

  // encode wifi command in buffer
  this->request_buffer_len =
      EncodeWiFiCommand(&wifi_cmd, request_buffer, sizeof(request_buffer));
}

void ModuleWiFi::CheckRequest(const Esp32Command &cmd) {
  Log.traceln("ModuleWiFi::CheckRequest");

  WiFiCommand wifi_cmd = WiFiCommand_init_zero;
  wifi_cmd.type = WiFiCommand_Type_CHECK;

  // set url and port
  HttpClient resp_msg = dirtviz.GetResponse();

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

  request_buffer_len =
      EncodeWiFiCommand(&wifi_cmd, request_buffer, sizeof(request_buffer));
}

void ModuleWiFi::NtpSync(const Esp32Command &cmd) {
  Log.traceln("ModuleWiFi::NtpSync");

  WiFiCommand wifi_cmd = WiFiCommand_init_zero;
  wifi_cmd.type = WiFiCommand_Type_NTP_SYNC;

  // force update
  timeClient->forceUpdate();

  request_buffer_len =
      EncodeWiFiCommand(&wifi_cmd, request_buffer, sizeof(request_buffer));
}

void ModuleWiFi::Time(const Esp32Command &cmd) {
  Log.traceln("ModuleWiFi::Time");

  WiFiCommand wifi_cmd = WiFiCommand_init_zero;
  wifi_cmd.type = WiFiCommand_Type_TIME;

  if (timeClient->isTimeSet()) {
    wifi_cmd.ts = timeClient->getEpochTime();
    Log.noticeln("Current timestamp: %d", wifi_cmd.ts);
  } else {
    Log.errorln("Failed to get time from NTP server!");
  }

  request_buffer_len =
      EncodeWiFiCommand(&wifi_cmd, request_buffer, sizeof(request_buffer));
}

void ModuleWiFi::Disconnect(const Esp32Command &cmd) {
  // init return command
  WiFiCommand wifi_cmd = WiFiCommand_init_zero;
  wifi_cmd.type = WiFiCommand_Type_CONNECT;

  Log.traceln("ModuleWiFi::Disconnect");

  timeClient->end();
  WiFi.disconnect();

  request_buffer_len =
      EncodeWiFiCommand(&wifi_cmd, request_buffer, sizeof(request_buffer));
}

void ModuleWiFi::CheckWiFi(const Esp32Command &cmd) {
  Log.traceln("ModuleWiFi::CheckWiFi");

  WiFiCommand wifi_cmd = WiFiCommand_init_zero;
  wifi_cmd.type = WiFiCommand_Type_CHECK_WIFI;

  // check if connected to WiFi connected
  int wifi_status = WiFi.status();
  wifi_cmd.rc = wifi_status;
  Log.noticeln("WiFi status: %d", wifi_status);
  if (wifi_status == WL_CONNECTED) {
    Log.noticeln("IP Address: %p", WiFi.localIP());
    Log.noticeln("Gateway IP: %p", WiFi.gatewayIP());
    Log.noticeln("Subnet Mask: %p", WiFi.subnetMask());
    Log.noticeln("DNS: %p", WiFi.dnsIP());

    // start NTP
    timeClient->begin();
  }

  request_buffer_len =
      EncodeWiFiCommand(&wifi_cmd, request_buffer, sizeof(request_buffer));
}

void ModuleWiFi::CheckApi(const Esp32Command &cmd) {
  Log.traceln("ModuleWiFi::CheckApi");

  WiFiCommand wifi_cmd = WiFiCommand_init_zero;
  wifi_cmd.type = WiFiCommand_Type_CHECK_API;

  // check API health
  dirtviz.SetUrl(cmd.command.wifi_command.url);
  dirtviz.Check();

  request_buffer_len =
      EncodeWiFiCommand(&wifi_cmd, request_buffer, sizeof(request_buffer));
}

void ModuleWiFi::Host(const Esp32Command &cmd) {
  Log.traceln("ModuleWiFi::Host");

  WiFiCommand resp = WiFiCommand_init_zero;
  resp.type = WiFiCommand_Type_HOST;
  
  // append wifi mac to the ssid to make unique
  std::string ssid = cmd.command.wifi_command.ssid;
  std::string pass = cmd.command.wifi_command.passwd;

  WiFi.softAP(ssid.c_str(), pass.c_str());
 
  std::string ip = WiFi.softAPIP().toString().c_str();
  Log.noticeln("Access Point started");

  request_buffer_len =
      EncodeWiFiCommand(&resp, request_buffer, sizeof(request_buffer));
}

void ModuleWiFi::StopHost() {
  Log.traceln("ModuleWiFi::StopHost");

  WiFiCommand resp = WiFiCommand_init_zero;
  resp.type = WiFiCommand_Type_STOP_HOST;

  // stop hosting access point
  WiFi.softAPdisconnect();

  request_buffer_len =
      EncodeWiFiCommand(&resp, request_buffer, sizeof(request_buffer));
}

void ModuleWiFi::HostInfo() {
  Log.traceln("ModuleWiFi::HostInfo");

  WiFiCommand resp = WiFiCommand_init_zero;
  resp.type = WiFiCommand_Type_HOST_INFO;

  // copy the ip address
  strncpy(resp.ssid, WiFi.softAPSSID().c_str(), sizeof(resp.ssid));
  strncpy(resp.url, WiFi.softAPIP().toString().c_str(), sizeof(resp.url));
  strncpy(resp.mac, WiFi.softAPmacAddress().c_str(), sizeof(resp.mac));

  Log.noticeln("SSID: %s", resp.ssid);
  Log.noticeln("IP: %s", resp.url);
  Log.noticeln("MAC: %s", resp.mac);

  request_buffer_len =
      EncodeWiFiCommand(&resp, request_buffer, sizeof(request_buffer));
}
