#include "modules/wifi.hpp"

ModuleWiFi::ModuleWiFi(void) {}

ModuleWiFi::~ModuleWiFi(void) {}

void ModuleWiFi::OnReceive(const Esp32Command& cmd) {
  // check if WiFi command
  if (cmd.which_command != Esp32Command_wifi_command_tag) {
    return;
  }

  // switch for command types
  switch (cmd.command.wifi_command.type)
  {
  case WiFiCommand_Type_CONNECT:
    this->Connect(cmd);
    break;

  case WiFiCommand_Type_POST:
    this->Post(cmd);
    break;
  
  default:
    break;
  }
}

size_t ModuleWiFi::OnRequest(uint8_t* buffer) {

}

void ModuleWiFi::Connect(const Esp32Command& cmd) {
  // connect to WiFi
  WiFi.begin(cmd.command.wifi_command.ssid, cmd.command.wifi_command.passwd);

  // wait for WiFi to connect
  unsigned long timeout = millis() + connect_timeout_ms;
  while (WiFi.status() != WL_CONNECTED) {
    // TODO implement lp sleep
    delay(500);
    Serial.print(".");

    // wifi connect timeout
    if (millis() >= timeout) {
      Serial.print("Could not connect to WiFi, SSID: ");
      Serial.print(cmd.command.wifi_command.ssid);
      Serial.print(", PASS: ");
      Serial.println(cmd.command.wifi_command.passwd);
    }
  }

  // print ip address
  Serial.println("");
  Serial.println("Connected!");
  Serial.println(WiFi.localIP());

  // set url
  this->dirtviz.SetUrl(cmd.command.wifi_command.url);
  this->dirtviz.SetPort(cmd.command.wifi_command.port);

  // TODO Add API status check
}

void ModuleWiFi::Post(const Esp32Command& cmd) {
  // send measurement
  const uint8_t* meas = cmd.command.wifi_command.resp.bytes;
  const size_t meas_len = cmd.command.wifi_command.resp.size;
  int status_code = this->dirtviz.SendMeasurement(meas, meas_len);

  // get response data
  // TODO may need to pass pointer of a pointer
  const uint8_t* resp = {};
  size_t resp_len = this->dirtviz.GetResponse(resp);

  // encode wifi command in buffer
  this->request_buffer_len = EncodeWiFiCommand(
    WiFiCommand_Type_POST,
    nullptr,
    nullptr,
    nullptr,
    0,
    status_code,
    0,
    resp,
    resp_len,
    *this->request_buffer,
    sizeof(*this->request_buffer)
  );
}