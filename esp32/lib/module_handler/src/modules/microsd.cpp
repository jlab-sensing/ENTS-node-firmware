#include "modules/microsd.hpp"

#include <ArduinoLog.h>
#include <SD.h>

#include "pb_decode.h"

#define MICROSD_FAT32_FILENAME_MAX_LENGTH 255

static const uint8_t chipSelect_pin = 7;
static UserConfiguration uc = UserConfiguration_init_zero;

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
  File dataFile;
  char dataFileFilename[MICROSD_FAT32_FILENAME_MAX_LENGTH + 1];
  char writeBuffer[256];

  MicroSDCommand return_cmd = MicroSDCommand_init_zero;
  return_cmd.type = MicroSDCommand_Type_SAVE;
  return_cmd.rc = MicroSDCommand_ReturnCode_SUCCESS;

  Log.traceln("ModuleMicroSD::Save");

  Log.verbose("microsd_command.filename: %s\r\n",
              cmd.command.microsd_command.filename);

  Log.verbose("microsd_command.type: %u\r\n", cmd.command.microsd_command.type);

  // Check payload type: encoded measurement or raw data
  // if (cmd.command.microsd_command.which_data == MicroSDCommand_meas_tag) {
  //   Log.verbose("Encoded Measurement\r\n");
  // } else if (cmd.command.microsd_command.which_data ==
  //            MicroSDCommand_raw_data_tag) {
  //   Log.verbose("Raw Data\r\n");
  // } else {
  //   Log.error("Unexpected data type, aborting save to micro SD card.\r\n");
  //   return_cmd.rc = MicroSDCommand_ReturnCode_ERROR_GENERAL;
  //   snprintf((char *)return_cmd.data.raw_data.bytes,
  //            sizeof(return_cmd.data.raw_data.bytes),
  //            "[ModuleMicroSD::Save] Error: Unexpected data type in "
  //            "'which_data' field.\r\n");
  //   this->request_buffer_len = EncodeMicroSDCommand(&return_cmd,
  //   request_buffer,
  //                                                   sizeof(request_buffer));
  //   return;
  // }

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
    return_cmd.rc = MicroSDCommand_ReturnCode_ERROR_MICROSD_NOT_INSERTED;
    this->request_buffer_len = EncodeMicroSDCommand(&return_cmd, request_buffer,
                                                    sizeof(request_buffer));
    return;
  }
  // Note: SD.begin(chipSelect) assumes the default SCLK, MISO, MOSI pins.
  // For non-default pin assignments, call SPI.begin(SCLK, MISO, MOSI, CS)
  // prior to SD.begin(CS).
  SD.end();
  if (!SD.begin(chipSelect_pin)) {
    Log.error(
        "Failed to begin, make sure that a FAT32 formatted SD card is "
        "inserted. Aborting save to micro SD card.\r\n");
    return_cmd.rc = MicroSDCommand_ReturnCode_ERROR_FILE_SYSTEM_NOT_MOUNTABLE;
  }

  Log.verbose("microsd_command.which_data: %u ",
              cmd.command.microsd_command.which_data);
  if (cmd.command.microsd_command.which_data ==
      MicroSDCommand_sensor_measurement_tag) {
    Log.verbose("Encoded SensorMeasurement\r\n");
    if (cmd.command.microsd_command.data.sensor_measurement.has_meta) {
      Log.verbose(
          "sensor_measurement.meta.cell_id: %u\r\n",
          cmd.command.microsd_command.data.sensor_measurement.meta.cell_id);
      Log.verbose(
          "sensor_measurement.meta.logger_id: %u\r\n",
          cmd.command.microsd_command.data.sensor_measurement.meta.logger_id);
      Log.verbose("sensor_measurement.meta.ts: %u\r\n",
                  cmd.command.microsd_command.data.sensor_measurement.meta.ts);
    } else {
      Log.verbose(
          "sensor_measurement no metadata, data will be appended without "
          "timestamp.\r\n");
    }

    // Use provided filename or generate filename based on measurement type.
    if (cmd.command.microsd_command.filename[0] != '\0') {
      strncpy(dataFileFilename, cmd.command.microsd_command.filename,
              sizeof(dataFileFilename));
    } else {
      // Generate filename using enum value and enum name.
      snprintf(dataFileFilename, sizeof(dataFileFilename), "/%d_%s.csv",
               cmd.command.microsd_command.data.sensor_measurement.type,
               SensorType_name(
                   cmd.command.microsd_command.data.sensor_measurement.type));
    }

    // Prepare data to append
    switch (cmd.command.microsd_command.data.sensor_measurement.which_value) {
      case SensorMeasurement_unsigned_int_tag:
        snprintf(
            writeBuffer, sizeof(writeBuffer), "%u,%u,%u,%u\r\n",
            cmd.command.microsd_command.data.sensor_measurement.meta.ts,
            cmd.command.microsd_command.data.sensor_measurement.meta.cell_id,
            cmd.command.microsd_command.data.sensor_measurement.meta.logger_id,
            cmd.command.microsd_command.data.sensor_measurement.value
                .unsigned_int);
        break;
      case SensorMeasurement_signed_int_tag:
        snprintf(
            writeBuffer, sizeof(writeBuffer), "%u,%u,%u,%u\r\n",
            cmd.command.microsd_command.data.sensor_measurement.meta.ts,
            cmd.command.microsd_command.data.sensor_measurement.meta.cell_id,
            cmd.command.microsd_command.data.sensor_measurement.meta.logger_id,
            cmd.command.microsd_command.data.sensor_measurement.value
                .signed_int);
        break;
      case SensorMeasurement_decimal_tag:
        snprintf(
            writeBuffer, sizeof(writeBuffer), "%u,%u,%u,%.9lf\r\n",
            cmd.command.microsd_command.data.sensor_measurement.meta.ts,
            cmd.command.microsd_command.data.sensor_measurement.meta.cell_id,
            cmd.command.microsd_command.data.sensor_measurement.meta.logger_id,
            cmd.command.microsd_command.data.sensor_measurement.value.decimal);
        break;
      default:
        Log.error(
            "Unexpected SensorMeasurement value type, aborting save to micro "
            "SD card.\r\n");
        return_cmd.rc = MicroSDCommand_ReturnCode_ERROR_GENERAL;
        snprintf(
            (char *)return_cmd.data.raw_data.bytes,
            sizeof(return_cmd.data.raw_data.bytes),
            "[ModuleMicroSD::Save] Error: Unexpected SensorMeasurement type in "
            "'which_value' field.\r\n");
        this->request_buffer_len = EncodeMicroSDCommand(
            &return_cmd, request_buffer, sizeof(request_buffer));
        return;
        break;
    }

    // If the file does not yet exist, create it and write the row 1 headers.
    if (!SD.exists(dataFileFilename)) {
      dataFile = SD.open(dataFileFilename, FILE_WRITE, true);
      if (!dataFile) {
        Log.error("Failed to create '%s' with '%s'\r\n", dataFileFilename,
                  FILE_WRITE);
        return_cmd.rc = MicroSDCommand_ReturnCode_ERROR_FILE_NOT_OPENED;
        strncpy(return_cmd.filename, dataFileFilename,
                sizeof(return_cmd.filename));
        return;
      }
      dataFile.printf(
          "unix_time (s),cell_id,logger_id,%s\r\n",
          SensorType_name(
              cmd.command.microsd_command.data.sensor_measurement.type));
      dataFile.close();
    }

  } else if (cmd.command.microsd_command.which_data ==
             MicroSDCommand_raw_data_tag) {
    Log.verbose("Raw Data\r\n");
    // Use provided filename or use "/raw.csv".
    if (cmd.command.microsd_command.filename[0] != '\0') {
      strncpy(dataFileFilename, cmd.command.microsd_command.filename,
              sizeof(dataFileFilename));
    } else {
      snprintf(dataFileFilename, sizeof(dataFileFilename), "/raw.csv");
    }

    if (cmd.command.microsd_command.filename[0] == '\0') {
      Log.error(
          "No filename provided for raw data write, aborting save to micro SD "
          "card.\r\n");
      return_cmd.rc = MicroSDCommand_ReturnCode_ERROR_GENERAL;
      snprintf((char *)return_cmd.data.raw_data.bytes,
               sizeof(return_cmd.data.raw_data.bytes),
               "[ModuleMicroSD::Save] Error: No filename provided for raw data "
               "write.\r\n");
      this->request_buffer_len = EncodeMicroSDCommand(
          &return_cmd, request_buffer, sizeof(request_buffer));
      return;
    }

  } else {
    Log.error(
        "Unexpected which_data type, aborting save to micro SD card.\r\n");
    return_cmd.rc = MicroSDCommand_ReturnCode_ERROR_GENERAL;
    snprintf((char *)return_cmd.data.raw_data.bytes,
             sizeof(return_cmd.data.raw_data.bytes),
             "[ModuleMicroSD::Save] Error: Unexpected data type in "
             "'which_data' field.\r\n");
    this->request_buffer_len = EncodeMicroSDCommand(&return_cmd, request_buffer,
                                                    sizeof(request_buffer));
    return;
  }

  dataFile = SD.open(dataFileFilename, FILE_APPEND);
  if (!dataFile) {
    Log.error("Failed to open '%s' with '%s'\r\n", dataFileFilename,
              FILE_APPEND);
    return_cmd.rc = MicroSDCommand_ReturnCode_ERROR_FILE_NOT_OPENED;
    strncpy(return_cmd.filename, dataFileFilename, sizeof(return_cmd.filename));
    return;
  }

  Log.trace("Successfully opened '%s' with '%s'\r\n", dataFileFilename,
            FILE_APPEND);

  // Append new data
  if (cmd.command.microsd_command.which_data ==
      MicroSDCommand_sensor_measurement_tag) {
    dataFile.printf("%s", writeBuffer);
  } else if (cmd.command.microsd_command.which_data ==
             MicroSDCommand_raw_data_tag) {
    for (uint32_t i = 0; i < cmd.command.microsd_command.data.raw_data.size;
         i++) {
      dataFile.write(cmd.command.microsd_command.data.raw_data.bytes[i]);
    }
  }

  dataFile.close();
  Log.trace("Wrote to and closed '%s'\r\n", dataFileFilename);

  // encode command in buffer
  this->request_buffer_len =
      EncodeMicroSDCommand(&return_cmd, request_buffer, sizeof(request_buffer));
}

void ModuleMicroSD::UserConfig(const Esp32Command &cmd) {
  File userConfigFile;
  char userConfigFileFilename[MICROSD_FAT32_FILENAME_MAX_LENGTH + 1];

  MicroSDCommand return_cmd = MicroSDCommand_init_zero;
  return_cmd.type = MicroSDCommand_Type_USERCONFIG;

  Log.traceln("ModuleMicroSD::UserConfig");

  memcpy(&uc, &cmd.command.microsd_command.data.uc, sizeof(UserConfiguration));

  // Check for SD card
  if (!microsd_detect_card()) {
    Log.error("Aborting save to micro SD card.\r\n");
    return_cmd.rc = MicroSDCommand_ReturnCode_ERROR_MICROSD_NOT_INSERTED;
    return;
  }
  // Note: SD.begin(chipSelect) assumes the default SCLK, MISO, MOSI pins.
  // For non-default pin assignments, call SPI.begin(SCLK, MISO, MOSI, CS)
  // prior to SD.begin(CS).
  SD.end();
  if (!SD.begin(chipSelect_pin)) {
    Log.error(
        "Failed to begin, make sure that a FAT32 formatted SD card is "
        "inserted. Aborting save to micro SD card.\r\n");
    return_cmd.rc = MicroSDCommand_ReturnCode_ERROR_FILE_SYSTEM_NOT_MOUNTABLE;
    return;
  }

  // (Over)Write a file for the userConfig
  if (cmd.command.microsd_command.filename[0] != '\0') {
    strncpy(userConfigFileFilename, cmd.command.microsd_command.filename,
            sizeof(userConfigFileFilename));
  } else {
    snprintf(userConfigFileFilename, sizeof(userConfigFileFilename),
             "/userconfig.txt");
  }

  userConfigFile = SD.open(userConfigFileFilename, FILE_WRITE, true);
  if (!userConfigFile) {
    Log.error("Failed to open/create '%s' with '%s'\r\n",
              userConfigFileFilename, FILE_WRITE);
    return_cmd.rc = MicroSDCommand_ReturnCode_ERROR_FILE_NOT_OPENED;
    strncpy(return_cmd.filename, userConfigFileFilename,
            sizeof(return_cmd.filename));
    return;
  }

  // Write the userconfig to the file
  userConfigFile.printf("logger_id=%u\r\n", uc.logger_id);
  userConfigFile.printf("cell_id=%u\r\n", uc.cell_id);
  userConfigFile.printf("Upload_method=%u (%s)\r\n", uc.Upload_method,
                        Uploadmethod_name(uc.Upload_method));
  userConfigFile.printf("Upload_interval=%u\r\n", uc.Upload_interval);
  userConfigFile.printf("enabled_sensors_multiple_count=%u\r\n",
                        uc.enabled_sensors_multiple_count);
  for (int i = 0; i < uc.enabled_sensors_multiple_count; i++) {
    userConfigFile.printf("enabled_sensors_multiple[%d].enabled_sensor=%d (%s)\r\n", i, uc.enabled_sensors_multiple[i].enabled_sensor, EnabledSensor_name(uc.enabled_sensors_multiple[i].enabled_sensor));
    userConfigFile.printf("enabled_sensors_multiple[%d].cell_id=%d\r\n", i, uc.enabled_sensors_multiple[i].cell_id);
    userConfigFile.printf("enabled_sensors_multiple[%d].index=%d\r\n", i, uc.enabled_sensors_multiple[i].index);
  }
  userConfigFile.printf("Voltage_Slope=%.9lf\r\n", uc.Voltage_Slope);
  userConfigFile.printf("Voltage_Offset=%.9lf\r\n", uc.Voltage_Offset);
  userConfigFile.printf("Current_Slope=%.9lf\r\n", uc.Current_Slope);
  userConfigFile.printf("Current_Offset=%.9lf\r\n", uc.Current_Offset);
  userConfigFile.printf("WiFi_SSID=%s\r\n", uc.WiFi_SSID);
  userConfigFile.printf("WiFi_Password=%s\r\n", uc.WiFi_Password);
  userConfigFile.printf("API_Endpoint_URL=%s\r\n", uc.API_Endpoint_URL);
  userConfigFile.printf("API_Endpoint_Port=%u\r\n", uc.API_Endpoint_Port);

  userConfigFile.close();

  // encode command in buffer
  this->request_buffer_len =
      EncodeMicroSDCommand(&return_cmd, request_buffer, sizeof(request_buffer));
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