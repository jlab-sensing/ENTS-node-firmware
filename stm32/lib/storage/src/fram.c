#include "fram.h"

#include "fram_def.h"

// #define FRAM_FM24CL16B
// #define FRAM_MB85RC1MT

#if defined(FRAM_FM24CL16B) && defined(FRAM_MB85RC1MT)
#error Only one FRAM chip can be enabled
#elif defined(FRAM_FM24CL16B)
#include "fm24cl16b.h"
const FramInterfaceType FramInterface = {.WritePtr = Fm24cl16bWrite,
                                         .ReadPtr = Fm24cl16bRead,
                                         .size = fm24cl16b_size};
#elif defined(FRAM_MB85RC1MT)
#include "mb85rc1mt.h"
const FramInterfaceType FramInterface = {.WritePtr = Mb85rc1mtWrite,
                                         .ReadPtr = Mb85rc1mtRead,
                                         .size = mb85rc1mt_size};
#else
#error No FRAM chip enabled
#endif

FramStatus FramWrite(FramAddr addr, const uint8_t *data, size_t len) {
  // check size
  if (addr + len > FramSize()) {
    return FRAM_OUT_OF_RANGE;
  }

  return FramInterface.WritePtr(addr, data, len);
}

FramStatus FramRead(FramAddr addr, size_t len, uint8_t *data) {
  // check size
  if (addr + len > FramSize()) {
    return FRAM_OUT_OF_RANGE;
  }

  return FramInterface.ReadPtr(addr, len, data);
}

FramAddr FramSize(void) { return FramInterface.size; }

HAL_StatusTypeDef ConfigureSettings(configuration c) {
  HAL_StatusTypeDef status = HAL_OK;

  // TODO(GSOC student) implement user config write

  return status;
}

configuration ReadSettings(void) {
  configuration c = {};

  // TODO(GSOC student) implement user config read

  return c;
}

FramStatus FramDump(uint16_t linesize, uint8_t displayformat, uint8_t omitjunk,
                    uint8_t printdelay_ms, uint16_t startaddress,
                    uint16_t endaddress) {
  FramStatus returnstatus = FRAM_OK;

  const uint16_t startaddress_aligned =
      startaddress - (startaddress % linesize);
  const uint16_t endaddress_aligned =
      endaddress + (linesize - (endaddress % linesize)) - 1;

  // DUMP FRAM
  APP_PRINTF("\r\n\r\nDumping FRAM...\r\n\r\n");

  if (omitjunk == DUMP_FRAM_OMIT_JUNK) {
    APP_PRINTF(
        "Lines with only junk data (i.e. 1 2 3 4 5...) are omitted.\r\n\r\n");
  } else {
    APP_PRINTF(
        "Omissions disabled, all selected memory will be printed.\r\n\r\n");
  }

  if (displayformat == DUMP_FRAM_DISPLAY_HEX) {
    APP_PRINTF("Memory values in HEX.\r\n\r\n");
  } else if (displayformat == DUMP_FRAM_DISPLAY_DECIMAL) {
    APP_PRINTF("Memory values in DECIMAL.\r\n\r\n");
  }

  APP_PRINTF(
      "Start: %5X (%6d) [round down to %5X (%6d)]\r\nEnd: %5X (%6d) [round up "
      "to %5X (%6d)]\r\n\r\n",
      startaddress, startaddress, startaddress_aligned, startaddress_aligned,
      endaddress, endaddress, endaddress_aligned, endaddress_aligned);

  if (startaddress > endaddress) {
    APP_PRINTF(
        "ERROR: startaddress must be less than or equal to endaddress.\r\n");
    return FRAM_ERROR;
  }
  if ((startaddress > (FramSize() - 1)) || (endaddress > (FramSize() - 1))) {
    APP_PRINTF(
        "ERROR: start and end address may not exceed memory capacity (maximum "
        "address is FramSize()-1 == %d).\r\n",
        FramSize() - 1);
    return FRAM_ERROR;
  }

  APP_PRINTF(
      "Anticipated runtime %d ms based on %d ms print delay, %d linesize, and "
      "%d bytes of memory selected.\r\n\r\n",
      (endaddress_aligned - startaddress_aligned) * printdelay_ms / linesize,
      printdelay_ms, linesize, endaddress_aligned - startaddress_aligned);

  // print column headers based on linesize
  APP_PRINTF("               ");
  for (uint16_t i = 0; i < linesize; i++) {
    if (displayformat == DUMP_FRAM_DISPLAY_HEX) {
      APP_PRINTF("+%2X ", i);
    } else if (displayformat == DUMP_FRAM_DISPLAY_DECIMAL) {
      APP_PRINTF("+%3d ", i);
    }
  }
  APP_PRINTF("\r\n");
  // FRAM chip is 1<<17 bytes long (131072).
  // linesize should be a factor of 1<<17
  uint8_t framdumpbuffer[linesize];
  FramStatus framdumpstatus;
  uint8_t printcurrentline, printcurrentline_prev = 1;
  uint8_t patternmatch;
  uint16_t omittedlinestart = 0;
  for (int line = startaddress_aligned / linesize;
       line <= (endaddress_aligned / linesize); line++) {
    framdumpstatus = FramRead(line * linesize, linesize, framdumpbuffer);
    if (framdumpstatus != FRAM_OK) {
      APP_PRINTF("\r\n%5X (%6d)  FramRead error: %d, continuing...",
                 line * linesize, line * linesize, framdumpstatus);
      returnstatus = framdumpstatus;
      HAL_Delay(printdelay_ms);
    } else {
      // assume the line contains useful data, and test to prove if it is
      // instead junk (and should not be printed)
      printcurrentline = 1;

      // pattern check: repeating (4 4 4 4 ...)
      if (printcurrentline == 1) {
        // assume the pattern matches
        patternmatch = 1;
        for (int offset = 0; offset < (linesize - 1); offset++) {
          if (framdumpbuffer[offset] != (framdumpbuffer[offset + 1])) {
            // pattern does not match, possibly useful data
            patternmatch = 0;
            break;
          }
        }
        if (patternmatch == 1) {
          // pattern matches, junk data
          printcurrentline = 0;
        }
      }

      // pattern check: incrementing +1 sequence (1 2 3 4 5 6 7 8 9 ...)
      if (printcurrentline == 1) {
        // assume the pattern matches
        patternmatch = 1;
        for (int offset = 0; offset < (linesize - 1); offset++) {
          if ((framdumpbuffer[offset] + 1) != framdumpbuffer[offset + 1]) {
            // pattern does not match, possibly useful data
            patternmatch = 0;
            break;
          }
        }
        if (patternmatch == 1) {
          // pattern matches, junk data
          printcurrentline = 0;
        }
      }

      // pattern check: 1 through 5 repeating, used in
      // stm32/test/test_fram/test_fram.c unit test "test_FramWrite_All" (1 2 3
      // 4 5 1 2 3 4 5 ...)
      if ((printcurrentline == 1) && (linesize > 5)) {
        // assume the pattern matches
        patternmatch = 1;
        for (int offset = 0; offset < (linesize - 5); offset++) {
          if (framdumpbuffer[offset] != framdumpbuffer[offset + 5]) {
            // pattern does not match, possibly useful data
            patternmatch = 0;
            break;
          }
        }
        if (patternmatch == 1) {
          // pattern matches, junk data
          printcurrentline = 0;
        }
      }

      if (printcurrentline == 1 || omitjunk == DUMP_FRAM_OMIT_NONE) {
        // if (printcurrentline_prev == 0)
        // {
        //   APP_PRINTF("\r\n%5X (%6d) [omit]", omittedlinestart * linesize,
        //   omittedlinestart * linesize);
        // }
        APP_PRINTF("\r\n%5X (%6d)  ", line * linesize, line * linesize);
        for (int offset = 0; offset < linesize; offset++) {
          if (displayformat == DUMP_FRAM_DISPLAY_HEX) {
            APP_PRINTF("%02X  ", framdumpbuffer[offset]);
          } else if (displayformat == DUMP_FRAM_DISPLAY_DECIMAL) {
            APP_PRINTF("%3d  ", framdumpbuffer[offset]);
          }
        }
        HAL_Delay(printdelay_ms);
      } else {
        if (printcurrentline_prev == 1) {
          omittedlinestart = line;
          APP_PRINTF("...\r\n");
        }
      }

      printcurrentline_prev = printcurrentline;
    }
  }
  APP_PRINTF("\r\n\r\nDone dumping FRAM.\r\n\r\n");

  return returnstatus;
}
