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
  if (addr + len >= FramSize()) {
    return FRAM_OUT_OF_RANGE;
  }

  return FramInterface.WritePtr(addr, data, len);
}

FramStatus FramRead(FramAddr addr, size_t len, uint8_t *data) {
  // check size
  if (addr + len >= FramSize()) {
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
  configuration c;

  // TODO(GSOC student) implement user config read

  return c;
}
