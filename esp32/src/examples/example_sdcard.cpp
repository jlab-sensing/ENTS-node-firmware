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

// The ESP32-C3 IO7 pins:

// IO4: CLK / SCK / SCLK
// IO5: SDO / DO / MISO
// IO6: SDI / DI / MOSI
// IO7: CS / SS
const uint8_t chipSelect = 7;
// IO10: CD "card detect" (for insertion detection).
// Note: Pin is floating when no card is inserted, grounded when a card is
// inserted.
const uint8_t cardDetect = 10;

File dataFile;

const String filename = "/data.csv";

void printCardInfo(void);
void printFileInfo(File f);
void printFileContents(File f);

void setup() {
  Serial.begin(115200);

  Serial.printf(
      "ents-node esp32 example_sdcard firmware, compiled at %s %s\r\n",
      __DATE__, __TIME__);
  Serial.printf("Git SHA: %s\r\n\r\n\r\n", GIT_REV);

  int cardIsInserted;
  pinMode(cardDetect, INPUT_PULLUP);
  cardIsInserted = digitalRead(cardDetect);
  if (cardIsInserted == LOW) {
    Serial.printf("Card detected (INPUT_PULLUP): %d\r\n", cardIsInserted);
  } else {
    Serial.printf("Card NOT detected (INPUT_PULLUP): %d\r\n", cardIsInserted);
  }

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

  Serial.printf("Checking file existence of '%s'\r\n", filename);
  if (SD.exists(filename)) {
    Serial.printf("'%s' exists.\r\n", filename);
  } else {
    Serial.printf("'%s' does not exist.\r\n", filename);
  }

  Serial.printf("Opening '%s' with '%s'\r\n", filename, FILE_READ);
  dataFile = SD.open(filename, FILE_READ);
  if (dataFile) {
    Serial.printf("Successfully opened '%s' with '%s'\r\n", filename,
                  FILE_READ);
    printFileInfo(dataFile);
    // printFileContents(dataFile);
    Serial.printf("Closing '%s'\r\n", filename);
    dataFile.close();
    Serial.printf("Closed '%s'\r\n", filename);
  } else {
    Serial.printf("Error opening '%s' with '%s'\r\n", filename, FILE_READ);
  }

  Serial.printf("Opening '%s' with '%s'\r\n", filename, FILE_APPEND);
  dataFile = SD.open(filename, FILE_APPEND);  // FILE_WRITE
  if (dataFile) {
    Serial.printf("Successfully opened '%s' with '%s'\r\n", filename,
                  FILE_APPEND);
    printFileInfo(dataFile);
    Serial.printf("Writing '%s' to file.\r\n", dataString);
    dataFile.printf("%s\r\n", dataString);
    Serial.printf("Closing '%s'\r\n", filename);
    dataFile.close();
    Serial.printf("Closed '%s'\r\n", filename);
  } else {
    Serial.printf("Error opening '%s' with '%s'\r\n", filename, FILE_APPEND);
  }

  Serial.printf("Opening '%s' with '%s'\r\n", filename, FILE_READ);
  dataFile = SD.open(filename, FILE_READ);
  if (dataFile) {
    Serial.printf("Successfully opened '%s' with '%s'\r\n", filename,
                  FILE_READ);
    printFileInfo(dataFile);
    printFileContents(dataFile);
    Serial.printf("Closing '%s'\r\n", filename);
    dataFile.close();
    Serial.printf("Closed '%s'\r\n", filename);
  } else {
    Serial.printf("Error opening '%s' with '%s'\r\n", filename, FILE_READ);
  }
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
  // char c = f.peek();
  // Serial.printf("peek: %c (0x%02x)\r\n", c, c);
  Serial.printf("position: %zd\r\n", f.position());
  Serial.printf("size: %zd\r\n", f.size());
  Serial.printf("-----File info END-----\r\n\r\n");
}

void printFileContents(File f) {
  Serial.printf("\r\n-----%s START-----\r\n", filename);
  char c = '\n';
  uint32_t line = 0;
  do {
    if (c == '\n') {
      Serial.printf("[%d]:\t", line++);
    }
    c = f.read();
    Serial.write(c);
  } while (f.available());
  Serial.printf("\r\n-----%s END-----\r\n", filename);
}