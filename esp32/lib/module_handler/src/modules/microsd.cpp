#include "modules/microsd.hpp"

#include <ArduinoLog.h>
#include <SD.h>

#include "pb_decode.h"

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
    case MicroSDCommand_Type_TIME:
      Log.traceln("Calling TIME");
      Time(cmd);
      break;
    case MicroSDCommand_Type_SIZE:
      Log.traceln("Calling SIZE");
      Size(cmd);
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
  // init return microSD command
  MicroSDCommand microsd_cmd = MicroSDCommand_init_zero;
  microsd_cmd.type = MicroSDCommand_Type_SAVE;

  Log.traceln("ModuleMicroSD::Save");

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
  if (!microsd_detect_card()) {
    Log.error("Aborting save to micro SD card.\r\n");
  }

  // Note: SD.begin(chipSelect) assumes the default SCLK, MISO, MOSI pins.
  // For non-default pin assignments, call SPI.begin(SCLK, MISO, MOSI, CS)
  // prior to SD.begin(CS).
  if (!SD.begin(chipSelect_pin)) {
    Log.error(
        "Failed to begin, make sure that a FAT32 formatted SD card is "
        "inserted. Aborting save to micro SD card.\r\n");
    return;
  }

  printCardInfo();

  static char filename[255];
  strncpy(filename, cmd.command.microsd_command.filename, 255);

  Measurement meas;
  if (!DecodeMeasurement(&meas,
                         (uint8_t *)cmd.command.microsd_command.resp.bytes,
                         cmd.command.microsd_command.resp.size)) {
    Log.error("Failed to decode measurement. Aborting.\r\n");
    return;
  }

  return;
  //
  // TODO: selectively insert column headers per received and decoded measurement types.
  //

  // if (filename[0] == '/0') {
  //   sprintf(filename, "%u.csv", cmd.command.microsd_command.ts);
  //   Log.notice(
  //       "First execution of ModuleMicroSD::Save, generating filename based on
  //       " "initial timestamp: %s\r\n", filename);
  // }

  Log.trace("Checking file existence of '%s'\r\n", filename);
  if (!SD.exists(filename)) {
    Log.trace("'%s' does not exist, creating file: %s\r\n", filename, filename);

    Log.trace("WRITING COLUMN HEADERS: Opening '%s' with '%s'\r\n", filename,
              FILE_WRITE);
    dataFile = SD.open(filename, FILE_WRITE);
    if (dataFile) {
      Log.trace("Successfully opened '%s' with '%s'\r\n", filename, FILE_WRITE);
      printFileInfo(dataFile);

      Measurement meas;
      pb_istream_t istream =
          pb_istream_from_buffer(cmd.command.microsd_command.resp.bytes,
                                 cmd.command.microsd_command.resp.size);
      if (!pb_decode(&istream, Measurement_fields, &meas)) {
        Log.error("Failed to decode. Aborting save to micro SD card.\r\n");
        return;
      }

      char headerString[50];
      switch (meas.which_measurement) {
        case Measurement_power_tag:
          snprintf(headerString, sizeof(headerString), "%s\r\n", "timestamp");
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

      Log.trace("Writing '%s' to file.\r\n", headerString);
      dataFile.printf("%s\r\n", headerString);

      Log.trace("Closing '%s'\r\n", filename);
      dataFile.close();
      Log.trace("Closed '%s'\r\n", filename);
    } else {
      Log.error("WRITING COLUMN HEADERS: Error opening '%s' with '%s'\r\n",
                filename, FILE_WRITE);
      Log.error("Aborting save to micro SD card.\r\n");
      return;
    }
  } else {
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
  }

  Log.trace("DURING WRITE: Opening '%s' with '%s'\r\n", filename, FILE_APPEND);
  dataFile = SD.open(filename, FILE_APPEND);
  if (dataFile) {
    Log.trace("Successfully opened '%s' with '%s'\r\n", filename, FILE_APPEND);
    // printFileInfo(dataFile);

    Measurement meas;
    pb_istream_t istream =
        pb_istream_from_buffer(cmd.command.microsd_command.resp.bytes,
                               cmd.command.microsd_command.resp.size);
    if (!pb_decode(&istream, Measurement_fields, &meas)) {
      Log.error("Failed to decode. Aborting save to micro SD card.\r\n");
      return;
    }

    char dataString[50];
    switch (meas.which_measurement) {
      case Measurement_power_tag:
        snprintf(dataString, sizeof(dataString), "%u,%u,%u\r\n",
                 cmd.command.microsd_command.ts, meas.measurement.power.voltage,
                 meas.measurement.power.current);
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

  // encode wifi command in buffer
  // this->request_buffer_len =
  //     EncodeWiFiCommand(&wifi_cmd, request_buffer, sizeof(request_buffer));
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