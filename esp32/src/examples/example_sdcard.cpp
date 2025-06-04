/**
 * @file example_sdcard.cpp
 * @author Jack Lin <jlin143@ucsc.edu>
 * @brief Example program for interfacing with an SD card.
 * @date 2025-05-22
 *
 * @copyright Copyright (c) 2024
 *
 */

#include <Arduino.h>
#include <SD.h>

// The ESP32-C3 IO7 pin is used as the slave select pin.
const uint8_t chipSelect = 7;

// IO4: CLK / SCK / SCLK
// IO5: SDO / DO / MISO
// IO6: SDI / DI / MOSI
// IO7: CS / SS
// IO10: CD "card detect" (for insertion detection)

File dataFile;

const String filename = "/data.csv";

void printCardInfo(void);
void printFileInfo(File f);

void setup() {
  Serial.begin(115200);

  Serial.printf(
      "ents-node esp32 example_sdcard firmware, compiled at %s %s\r\n",
      __DATE__, __TIME__);
  Serial.printf("Git SHA: %s\r\n\r\n\r\n", GIT_REV);

  // Note: SD.begin(chipSelect) assumes the default SCLK, MISO, MOSI pins.
  // For non-default pin assignments, call SPI.begin(SCLK, MISO, MOSI, CS) prior
  // to SD.begin(CS).
  if (!SD.begin(chipSelect)) {
    Serial.printf(
        "Failed to begin, make sure that a FAT32 formatted SD card is "
        "inserted.\r\n");
    while (1);
  }

  printCardInfo();

  String dataString = "";
  for (int i = 0; i < 3; i++) {
    // int sensor = analogRead(i);
    dataString += String(i);
    if (i < 2) {
      dataString += ",";
    }
  }
  Serial.printf("dataString to be appended: %s\r\n", dataString);

  if (SD.exists(filename)) {
    Serial.printf("Found '%s' (opening with '%s')\r\n", filename, FILE_READ);

    dataFile = SD.open(filename, FILE_READ);

    printFileInfo(dataFile);
  } else {
    Serial.printf("Did not find '%s'\r\n", filename);
  }

  Serial.printf("Opening '%s' with '%s'\r\n", filename, FILE_APPEND);
  dataFile = SD.open(filename, FILE_APPEND);  // FILE_WRITE

  if (dataFile) {
    printFileInfo(dataFile);
    Serial.printf("Writing '%s' to file.\r\n", dataString);
    dataFile.printf("%s\r\n", dataString);
    dataFile.close();
  } else {
    Serial.printf("Error opening '%s' with '%s'\r\n", filename, FILE_APPEND);
  }

  dataFile = SD.open(filename, FILE_READ);
  if (dataFile) {
    printFileInfo(dataFile);

    Serial.printf("\r\n-----%s START-----\r\n", filename);
    char c = '\n';
    uint32_t line = 0;
    do {
      if (c == '\n') {
        Serial.printf("[%d]:\t", line++);
      }
      c = dataFile.read();
      Serial.write(c);
    } while (dataFile.available());
    dataFile.close();
    Serial.printf("\r\n-----%s END-----\r\n", filename);
  } else {
    Serial.printf("error opening '%s' with '%s'\r\n", filename, FILE_READ);
  }
  while (1);
}

void loop() {}

void printCardInfo(void) {
  Serial.printf("\r\n-----Card info START-----\r\n");
  Serial.printf("sectors, sector size, total size: %zd * %zd = %llu\r\n",
                SD.numSectors(), SD.sectorSize(), SD.cardSize());
  String sdcard_type_t_strings[] = {"CARD_NONE", "CARD_MMC", "CARD_SD",
                                    "CARD_SDHC", "CARD_UNKNOWN"};
  Serial.printf("card type: (%d) %s\r\n", SD.cardType(),
                sdcard_type_t_strings[SD.cardType()]);
  Serial.printf("bytes used / total: %llu / %llu\r\n", SD.usedBytes(),
                SD.totalBytes());
  Serial.printf("-----Card info END-----\r\n\r\n");
}

void printFileInfo(File f) {
  Serial.printf("\r\n-----File info START-----\r\n");
  Serial.printf("available: %d\r\n", f.available());
  Serial.printf("timeout: %lu\r\n", f.getTimeout());
  Serial.printf("name: %s\r\n", f.name());
  Serial.printf("path: %s\r\n", f.path());
  char c = f.peek();
  Serial.printf("peek: %d (%02x) (%c)\r\n", c, c, c);
  Serial.printf("position: %zd\r\n", f.position());
  Serial.printf("size: %zd\r\n", f.size());
  Serial.printf("-----File info END-----\r\n\r\n");
}