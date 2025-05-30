#include "modules/wifi.hpp"

#include <ArduinoLog.h>
#include <SD.h>

#include "http.hpp"
#include "pb_decode.h"

static void printCardInfo(void);
static void printFileInfo(File f);
static void printFileContents(File f);

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

    case WiFiCommand_Type_SAVE_MICROSD_CARD:
      Log.traceln("Calling SAVE_MICROSD_CARD");
      Save(cmd);
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

void ModuleWiFi::Save(const Esp32Command &cmd) {
  // init return WiFi command
  WiFiCommand wifi_cmd = WiFiCommand_init_zero;
  wifi_cmd.type = WiFiCommand_Type_SAVE_MICROSD_CARD;

  Log.traceln("ModuleWiFI::Save");

  File dataFile;

  // The ESP32-C3 IO7 pins:
  // IO4: CLK / SCK / SCLK
  // IO5: SDO / DO / MISO
  // IO6: SDI / DI / MOSI
  // IO7: CS / SS
  const uint8_t chipSelect_pin = 7;
  // IO10: CD "card detect" (for insertion detection).
  // Note: Pin is floating when no card is inserted, grounded when a card is
  // inserted.
  const uint8_t cardDetect_pin = 10;

  pinMode(cardDetect_pin, INPUT_PULLUP);
  if (digitalRead(cardDetect_pin) == LOW) {
    Log.trace("Card detected.\r\n");
  } else {
    Log.error("Card NOT detected, aborting save to micro SD card.\r\n");
    pinMode(cardDetect_pin, INPUT);  // After checking the SD card, revert to
                                     // the lowest power pin mode.
    return;
  }
  pinMode(cardDetect_pin, INPUT);  // After checking the SD card, revert to the
                                   // lowest power pin mode.

  // Note: SD.begin(chipSelect) assumes the default SCLK, MISO, MOSI pins.
  // For non-default pin assignments, call SPI.begin(SCLK, MISO, MOSI, CS) prior
  // to SD.begin(CS).
  if (!SD.begin(chipSelect_pin)) {
    Log.error(
        "Failed to begin, make sure that a FAT32 formatted SD card is "
        "inserted. Aborting save to micro SD card.\r\n");
    return;
  }

  printCardInfo();

  static char filename[50] = "";
  if (filename[0] == '/0') {
    sprintf(filename, "%u.csv", cmd.command.wifi_command.ts);
    Log.notice(
        "First execution of ModuleWiFI::Save, generating filename based on "
        "initial timestamp: %s\r\n",
        filename);
  }

  Log.trace("Checking file existence of '%s'\r\n", filename);
  if (SD.exists(filename)) {
    Log.trace("'%s' exists.\r\n", filename);

    // Optional read before writing to see if the file changes after writing.
    Log.trace("BEFORE WRITE: Opening '%s' with '%s'\r\n", filename, FILE_READ);
    dataFile = SD.open(filename, FILE_READ);
    if (dataFile) {
      Log.trace("Successfully opened '%s' with '%s'\r\n", filename, FILE_READ);
      printFileInfo(dataFile);
      // printFileContents(dataFile);
      Log.trace("Closing '%s'\r\n", filename);
      dataFile.close();
      Log.trace("Closed '%s'\r\n", filename);
    } else {
      Log.error("BEFORE WRITE: Error opening '%s' with '%s'\r\n", filename,
                FILE_READ);
    }

  } else {
    Log.trace("'%s' does not exist.\r\n", filename);

    // TO DO: Write column headers based on measurment sensors
  }

  Log.trace("DURING WRITE: Opening '%s' with '%s'\r\n", filename, FILE_APPEND);
  dataFile = SD.open(filename, FILE_APPEND);  // FILE_WRITE
  if (dataFile) {
    Log.trace("Successfully opened '%s' with '%s'\r\n", filename, FILE_APPEND);
    printFileInfo(dataFile);

    Measurement meas;
    pb_istream_t istream =
        pb_istream_from_buffer(cmd.command.wifi_command.resp.bytes,
                               cmd.command.wifi_command.resp.size);
    if (!pb_decode(&istream, Measurement_fields, &meas)) {
      Log.error("Failed to decode.\r\n");
    }

    char dataString[50];
    switch (meas.which_measurement) {
      case Measurement_power_tag:
        snprintf(dataString, sizeof(dataString), "%u,%u,%u\r\n", cmd.command.wifi_command.ts, meas.measurement.power.voltage, meas.measurement.power.current);
        break;
      case Measurement_teros12_tag:
        break;
      case Measurement_phytos31_tag:
        break;
      case Measurement_bme280_tag:
        break;
      case Measurement_teros21_tag:
        break;
      default:
        Log.error("Unrecognized measurement type: %d\r\n",
                  meas.which_measurement);
        break;
    }

    Log.trace("Writing '%s' to file.\r\n", dataString);
    dataFile.printf("%s\r\n", dataString);

    Log.trace("Closing '%s'\r\n", filename);
    dataFile.close();
    Log.trace("Closed '%s'\r\n", filename);
  } else {
    Log.error("DURING WRITE: Error opening '%s' with '%s'\r\n", filename,
              FILE_APPEND);
  }

  // Optional read after writing to verify the newest append.
  Log.trace("AFTER WRITE: Opening '%s' with '%s'\r\n", filename, FILE_READ);
  dataFile = SD.open(filename, FILE_READ);
  if (dataFile) {
    Log.trace("Successfully opened '%s' with '%s'\r\n", filename, FILE_READ);
    printFileInfo(dataFile);
    printFileContents(dataFile);
    Log.trace("Closing '%s'\r\n", filename);
    dataFile.close();
    Log.trace("Closed '%s'\r\n", filename);
  } else {
    Log.error("AFTER WRITE: Error opening '%s' with '%s'\r\n", filename,
              FILE_READ);
  }

  // check if micro SD card is connected
  if (WiFi.status() != WL_CONNECTED) {
    Log.errorln("Micro SD card not detected!");

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

static void printCardInfo(void) {
  Log.verbose("\r\n-----Card info START-----\r\n");
  Log.verbose("sectors, sector size, total size: %zd * %zd = %llu\r\n",
              SD.numSectors(), SD.sectorSize(), SD.cardSize());
  String sdcard_type_t_strings[] = {"CARD_NONE", "CARD_MMC", "CARD_SD",
                                    "CARD_SDHC", "CARD_UNKNOWN"};
  Log.verbose("card type: (%d) %s\r\n", SD.cardType(),
              sdcard_type_t_strings[SD.cardType()]);
  Log.verbose("bytes used / total: %llu / %llu\r\n", SD.usedBytes(),
              SD.totalBytes());
  Log.verbose("-----Card info END-----\r\n\r\n");
}

static void printFileInfo(File f) {
  Log.verbose("\r\n-----File info START-----\r\n");
  Log.verbose("available: %d\r\n", f.available());
  Log.verbose("timeout: %lu\r\n", f.getTimeout());
  Log.verbose("name: %s\r\n", f.name());
  Log.verbose("path: %s\r\n", f.path());
  Log.verbose("position: %zd\r\n", f.position());
  Log.verbose("size: %zd\r\n", f.size());
  Log.verbose("-----File info END-----\r\n\r\n");
}

static void printFileContents(File f) {
  Log.verbose("\r\n-----%s START-----\r\n", f.path());
  char c = '\n';
  uint32_t line = 0;
  do {
    if (c == '\n') {
      Log.verbose("[%d]:\t", line++);
    }
    c = f.read();
    Log.verbose("%c", c);
  } while (f.available());
  Log.verbose("\r\n-----%s END-----\r\n", f.path());
}