#ifndef PCAP02_STANDARD_H
#define PCAP02_STANDARD_H

#include <stdint.h>

// All addresses mentioned in this file are byte-addresses.

// Begin copying firmware to SRAM at the SRAM address PROGRAM_OFFSET.
#define PCAP02_STANDARD_FIRMWARE_PROGRAM_OFFSET 0
#define PCAP02_STANDARD_FIRMWARE_PROGRAM_LENGTH 1524

// The main program is offset by MAIN_OFFSET from PROGRAM_OFFSET.
#define PCAP02_STANDARD_FIRMWARE_MAIN_OFFSET 0
#define PCAP02_STANDARD_FIRMWARE_MAIN_LENGTH 364
// Likewise, the included headers are offset by INCLUDE_OFFSET from
// PROGRAM_OFFSET.
#define PCAP02_STANDARD_FIRMWARE_INCLUDE_OFFSET 512
#define PCAP02_STANDARD_FIRMWARE_INCLUDE_LENGTH 1011

// The firmware version data is not included in pcap02_standard_firmware[] and
// must be written separately (if desired). The firmware version is offset by
// VERSION_OFFSET from PROGRAM_OFFSET. Firmware version at offset +4008 (+0xFA8)
// {2 + FWT_Standard + FWG_Capacitance}
#define PCAP02_STANDARD_FIRMWARE_VERSION_OFFSET 4008
#define PCAP02_STANDARD_FIRMWARE_VERSION_LENGTH 3

extern const uint8_t pcap02_standard_firmware[];

extern const uint8_t pcap02_standard_firmware_version[];

// Note: Register 77 (RUNBIT) is included here.
extern uint8_t pcap02_standard_config_registers[];

// The standard firmware's result registers RES1 - RES7 store the output as
// 3.21 unsigned fixed point numbers. See Section 5.5.1 "Result Registers" of
// the datasheet.
#define PCAP02_STANDARD_FIRMWARE_RESULT_FIXED_BITS 3
#define PCAP02_STANDARD_FIRMWARE_RESULT_FRACTIONAL_BITS 21

typedef union {
  uint8_t byte[3];
  uint32_t word : 24;
  struct {
    uint32_t fractional : PCAP02_STANDARD_FIRMWARE_RESULT_FRACTIONAL_BITS;
    uint32_t fixed : PCAP02_STANDARD_FIRMWARE_RESULT_FIXED_BITS;
  };
} pcap02_result_t;

// Converts a 3.21 unsigned fixed point number to a float.
double fixed_to_double(pcap02_result_t *res);

#endif