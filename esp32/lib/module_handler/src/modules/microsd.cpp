#include "modules/microsd.hpp"

#include <ArduinoLog.h>
#include <SD.h>

#include "pb_decode.h"

static const uint8_t chipSelect_pin = 7;
static UserConfiguration uc = UserConfiguration_init_zero;
static char dataFileFilename[256];

static void printCardInfo(void);
static void printFileInfo(File f);
static void printFileContents(File f);
static bool microsd_detect_card(void);

ModuleMicroSD::ModuleMicroSD(void) {
  // set module type
  type = Esp32Command_microsd_command_tag;
}

ModuleMicroSD::~ModuleMicroSD(void) {}

void ModuleMicroSD::OnReceive(const Esp32Command &cmd) {
  Log.traceln("ModuleMicroSD::OnReceive");

  // check if microSD command
  if (cmd.which_command != Esp32Command_microsd_command_tag) {
    return;
  }

  Log.traceln("MicroSDCommand type: %d", cmd.command.microsd_command.type);

  // switch for command types
  switch (cmd.command.microsd_command.type) {
    case MicroSDCommand_Type_SAVE:
      Log.traceln("Calling SAVE");
      Save(cmd);
      break;
    case MicroSDCommand_Type_USERCONFIG:
      Log.traceln("Calling USERCONFIG");
      UserConfig(cmd);
      break;
    default:
      Log.warningln("MicroSD command type not found!");
      break;
  }
}

size_t ModuleMicroSD::OnRequest(uint8_t *buffer) {
  Log.traceln("ModuleMicroSD::OnRequest");
  memcpy(buffer, request_buffer, request_buffer_len);
  return request_buffer_len;
}

void ModuleMicroSD::Save(const Esp32Command &cmd) {
  static uint32_t last_ts = 0;
  // init return microSD command
  MicroSDCommand microsd_cmd = MicroSDCommand_init_zero;
  microsd_cmd.type = MicroSDCommand_Type_SAVE;

  Log.traceln("ModuleMicroSD::Save");

  Log.verbose("microsd_command.filename: %s\r\n",
              cmd.command.microsd_command.filename);
  Log.verbose("microsd_command.type: %u\r\n", cmd.command.microsd_command.type);
  if (cmd.command.microsd_command.which_data == MicroSDCommand_meas_tag) {
    Log.verbose("microsd_command.which_data: %u (%s)\r\n",
                cmd.command.microsd_command.which_data,
                "MicroSDCommand_meas_tag");
  } else if (cmd.command.microsd_command.which_data == MicroSDCommand_uc_tag) {
    Log.verbose("microsd_command.which_data: %u (%s)\r\n",
                cmd.command.microsd_command.which_data,
                "MicroSDCommand_uc_tag");
  } else {
    Log.verbose("microsd_command.which_data: %u (%s)\r\n",
                cmd.command.microsd_command.which_data, "UNKNOWN");
  }

  File dataFile;

  // The ESP32-C3 IO7 pins:
  // IO4: CLK / SCK / SCLK
  // IO5: SDO / DO / MISO
  // IO6: SDI / DI / MOSI
  // IO7: CS / SS
  // IO10: CD "card detect" (for insertion detection).
  // Note: Pin is floating when no card is inserted, grounded when a card is
  // inserted.
  if (!microsd_detect_card()) {
    Log.error("Aborting save to micro SD card.\r\n");
    microsd_cmd.rc = MicroSDCommand_ReturnCode_ERROR_MICROSD_NOT_INSERTED;
  } else {
    // Note: SD.begin(chipSelect) assumes the default SCLK, MISO, MOSI pins.
    // For non-default pin assignments, call SPI.begin(SCLK, MISO, MOSI, CS)
    // prior to SD.begin(CS).
    SD.end();
    if (!SD.begin(chipSelect_pin)) {
      Log.error(
          "Failed to begin, make sure that a FAT32 formatted SD card is "
          "inserted. Aborting save to micro SD card.\r\n");
      microsd_cmd.rc =
          MicroSDCommand_ReturnCode_ERROR_FILE_SYSTEM_NOT_MOUNTABLE;
    } else {
      {  // Meas log print
        if (cmd.command.microsd_command.data.meas.has_meta) {
          Log.verbose("meas.meta.cell_id: %u\r\n",
                      cmd.command.microsd_command.data.meas.meta.cell_id);
          Log.verbose("meas.meta.logger_id: %u\r\n",
                      cmd.command.microsd_command.data.meas.meta.logger_id);
          Log.verbose("meas.meta.ts: %u\r\n",
                      cmd.command.microsd_command.data.meas.meta.ts);
        }
        Log.verbose("meas.which_measurement: %u\r\n",
                    cmd.command.microsd_command.data.meas.which_measurement);
        switch (cmd.command.microsd_command.data.meas.which_measurement) {
          case Measurement_power_tag:
            Log.verbose(
                ",%lf,%lf",
                cmd.command.microsd_command.data.meas.measurement.power.voltage,
                cmd.command.microsd_command.data.meas.measurement.power
                    .current);
            break;
          case Measurement_teros12_tag:
            Log.verbose(
                ",%lf,%u,%lf",
                cmd.command.microsd_command.data.meas.measurement.teros12
                    .vwc_adj,
                cmd.command.microsd_command.data.meas.measurement.teros12.ec,
                cmd.command.microsd_command.data.meas.measurement.teros12.temp);
            break;
          case Measurement_phytos31_tag:
            Log.verbose(",%lf,%lf",
                        cmd.command.microsd_command.data.meas.measurement
                            .phytos31.voltage,
                        cmd.command.microsd_command.data.meas.measurement
                            .phytos31.leaf_wetness);
            break;
          case Measurement_bme280_tag:
            Log.verbose(",%u,%d,%u",
                        cmd.command.microsd_command.data.meas.measurement.bme280
                            .pressure,
                        cmd.command.microsd_command.data.meas.measurement.bme280
                            .temperature,
                        cmd.command.microsd_command.data.meas.measurement.bme280
                            .humidity);
            break;
          case Measurement_teros21_tag:
            Log.verbose(
                ",%lf,%lf",
                cmd.command.microsd_command.data.meas.measurement.teros21
                    .matric_pot,
                cmd.command.microsd_command.data.meas.measurement.teros21.temp);
            break;
          case Measurement_meta_tag:
            Log.verbose(
                "meta tag: %d\r\n",
                cmd.command.microsd_command.data.meas.which_measurement);
            break;
          default:
            Log.verbose(
                "Unrecognized measurement type: %d\r\n",
                cmd.command.microsd_command.data.meas.which_measurement);
            microsd_cmd.rc = MicroSDCommand_ReturnCode_ERROR_GENERAL;
            break;
        }
      }
      dataFile = SD.open(dataFileFilename, FILE_APPEND);
      if (!dataFile) {
        Log.error("Failed to open '%s' with '%s'\r\n", dataFileFilename,
                  FILE_APPEND);
        microsd_cmd.rc = MicroSDCommand_ReturnCode_ERROR_FILE_NOT_OPENED;
        strncpy(microsd_cmd.filename, dataFileFilename,
                sizeof(microsd_cmd.filename));
      } else {
        Log.trace("Successfully opened '%s' with '%s'\r\n", dataFileFilename,
                  FILE_APPEND);

        // If the new (current) measurement is on a new timestamp, create a
        // new line
        if (cmd.command.microsd_command.data.meas.meta.ts != last_ts) {
          dataFile.printf("\r\n%u",
                          cmd.command.microsd_command.data.meas.meta.ts);
        }

        // Note: Assume that the measurements are sent in ascending order of
        // timestamp and _tag (such that the transmission order matches the CSV
        // row 1 header order).
        switch (cmd.command.microsd_command.data.meas.which_measurement) {
          case Measurement_power_tag:
            dataFile.printf(
                ",%lf,%lf",
                cmd.command.microsd_command.data.meas.measurement.power.voltage,
                cmd.command.microsd_command.data.meas.measurement.power
                    .current);
            break;
          case Measurement_teros12_tag:
            dataFile.printf(
                ",%lf,%u,%lf",
                cmd.command.microsd_command.data.meas.measurement.teros12
                    .vwc_adj,
                cmd.command.microsd_command.data.meas.measurement.teros12.ec,
                cmd.command.microsd_command.data.meas.measurement.teros12.temp);
            break;
          // case Measurement_phytos31_tag:
          //   dataFile.printf(",%lf,%lf", meas.measurement.phytos31.voltage,
          //                   meas.measurement.phytos31.leaf_wetness);
          //   break;
          case Measurement_bme280_tag:
            dataFile.printf(",%u,%d,%u",
                            cmd.command.microsd_command.data.meas.measurement
                                .bme280.pressure,
                            cmd.command.microsd_command.data.meas.measurement
                                .bme280.temperature,
                            cmd.command.microsd_command.data.meas.measurement
                                .bme280.humidity);
            break;
          case Measurement_teros21_tag:
            dataFile.printf(
                ",%lf,%lf",
                cmd.command.microsd_command.data.meas.measurement.teros21
                    .matric_pot,
                cmd.command.microsd_command.data.meas.measurement.teros21.temp);
            break;
          default:
            Log.error("Unrecognized measurement type: %d\r\n",
                      cmd.command.microsd_command.data.meas.which_measurement);
            microsd_cmd.rc = MicroSDCommand_ReturnCode_ERROR_GENERAL;
            break;
        }

        dataFile.close();
        Log.trace("Wrote to and closed '%s'\r\n", dataFileFilename);

        last_ts = cmd.command.microsd_command.data.meas.meta.ts;
      }
    }
  }
  // encode command in buffer
  this->request_buffer_len = EncodeMicroSDCommand(&microsd_cmd, request_buffer,
                                                  sizeof(request_buffer));
}

void ModuleMicroSD::UserConfig(const Esp32Command &cmd) {
  // init return microSD command
  MicroSDCommand microsd_cmd = MicroSDCommand_init_zero;
  microsd_cmd.type = MicroSDCommand_Type_USERCONFIG;

  Log.traceln("ModuleMicroSD::UserConfig");

  memcpy(&uc, &cmd.command.microsd_command.data.uc, sizeof(UserConfiguration));

  // Check for SD card
  if (!microsd_detect_card()) {
    Log.error("Aborting save to micro SD card.\r\n");
    microsd_cmd.rc = MicroSDCommand_ReturnCode_ERROR_MICROSD_NOT_INSERTED;
  } else {
    // Note: SD.begin(chipSelect) assumes the default SCLK, MISO, MOSI pins.
    // For non-default pin assignments, call SPI.begin(SCLK, MISO, MOSI, CS)
    // prior to SD.begin(CS).
    SD.end();
    if (!SD.begin(chipSelect_pin)) {
      Log.error(
          "Failed to begin, make sure that a FAT32 formatted SD card is "
          "inserted. Aborting save to micro SD card.\r\n");
      microsd_cmd.rc =
          MicroSDCommand_ReturnCode_ERROR_FILE_SYSTEM_NOT_MOUNTABLE;
    } else {
      // (Over)Write a file for the userConfig
      char userConfigFileFilename[sizeof(dataFileFilename)] = {0};
      strncpy(userConfigFileFilename, cmd.command.microsd_command.filename,
              sizeof(userConfigFileFilename));
      strncat(userConfigFileFilename, ".userconfig", sizeof(".userconfig"));
      File userConfigFile = SD.open(userConfigFileFilename, FILE_WRITE);
      if (!userConfigFile) {
        Log.error("Failed to open '%s' with '%s'\r\n", userConfigFileFilename,
                  FILE_WRITE);
        microsd_cmd.rc = MicroSDCommand_ReturnCode_ERROR_FILE_NOT_OPENED;
        strncpy(microsd_cmd.filename, userConfigFileFilename,
                sizeof(microsd_cmd.filename));
      } else {
        // Write the userconfig to the file
        userConfigFile.printf("logger_id=%d\r\n", uc.logger_id);
        userConfigFile.printf("cell_id=%d\r\n", uc.cell_id);
        char upload_method_s[][18] = {"Uploadmethod_LoRa", "Uploadmethod_WiFi",
                                      "UNKNOWN"};
        userConfigFile.printf("Upload_method=%d (%s)\r\n", uc.Upload_method,
                              upload_method_s[uc.Upload_method]);

        userConfigFile.printf("Upload_interval=%d\r\n", uc.Upload_interval);
        userConfigFile.printf("enabled_sensors_count=%d\r\n",
                              uc.enabled_sensors_count);

        char enabled_sensors_s[][8] = {"Voltage", "Current", "Teros12",
                                       "Teros21", "BME280"};
        for (int i = 0; i < uc.enabled_sensors_count; i++) {
          userConfigFile.printf("enabled_sensors[%d]=%d (%s)\r\n", i,
                                uc.enabled_sensors[i], enabled_sensors_s[i]);
        }

        userConfigFile.printf("Voltage_Slope=%lf\r\n", uc.Voltage_Slope);
        userConfigFile.printf("Voltage_Offset=%lf\r\n", uc.Voltage_Offset);
        userConfigFile.printf("Current_Slope=%lf\r\n", uc.Current_Slope);
        userConfigFile.printf("Current_Offset=%lf\r\n", uc.Current_Offset);
        userConfigFile.printf("WiFi_SSID=%s\r\n", uc.WiFi_SSID);
        userConfigFile.printf("WiFi_Password=%s\r\n", uc.WiFi_Password);
        userConfigFile.printf("API_Endpoint_URL=%s\r\n", uc.API_Endpoint_URL);
        userConfigFile.printf("API_Endpoint_Port=%d\r\n", uc.API_Endpoint_Port);

        userConfigFile.close();

        // Create a new file for the CSV
        strncpy(dataFileFilename, cmd.command.microsd_command.filename,
                sizeof(dataFileFilename));

        File dataFile = SD.open(dataFileFilename, FILE_WRITE);
        if (!dataFile) {
          Log.error("Failed to open '%s' with '%s'\r\n", dataFileFilename,
                    FILE_WRITE);
          microsd_cmd.rc = MicroSDCommand_ReturnCode_ERROR_FILE_NOT_OPENED;
          strncpy(microsd_cmd.filename, dataFileFilename,
                  sizeof(microsd_cmd.filename));
        } else {
          // Add the headers
          dataFile.printf("timestamp");
          for (int i = 0; i < uc.enabled_sensors_count; i++) {
            switch (uc.enabled_sensors[i]) {
              case EnabledSensor_Voltage:
              case EnabledSensor_Current:
                dataFile.printf(",voltage,current");
                break;
              case EnabledSensor_Teros12:
                dataFile.printf(",vwc_teros12,ec_teros12,temp_teros12");
                break;
              case EnabledSensor_Teros21:
                dataFile.printf(",matricpotential_teros21,temp_teros21");
                break;
              case EnabledSensor_BME280:
                dataFile.printf(
                    ",pressure_bme280,temperature_bme280,humidity_bme280");
                break;
              default:
                dataFile.printf(",ERROR Unknown sensor type");
                microsd_cmd.rc = MicroSDCommand_ReturnCode_ERROR_GENERAL;
                break;
            }
          }
          dataFile.close();
        }
      }
    }
  }

  // encode command in buffer
  this->request_buffer_len = EncodeMicroSDCommand(&microsd_cmd, request_buffer,
                                                  sizeof(request_buffer));
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

static bool microsd_detect_card(void) {
  const uint8_t cardDetect_pin = 10;

  pinMode(cardDetect_pin, INPUT_PULLUP);
  int pinState = digitalRead(cardDetect_pin);
  pinMode(cardDetect_pin, INPUT);  // After checking the SD card, revert to
                                   // the lowest power pin mode.

  if (pinState == LOW) {
    Log.trace("Card detected.\r\n");
    return true;
  } else {
    Log.error("Card NOT detected.\r\n");
    return false;
  }
}