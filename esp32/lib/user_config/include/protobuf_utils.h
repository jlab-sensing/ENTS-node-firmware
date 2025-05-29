#ifndef PROTOBUF_UTILS_H
#define PROTOBUF_UTILS_H

#include <stdint.h>
#include "soil_power_sensor.pb.h"
#include "transcoder.h"

void printEncodedData(const uint8_t *data, size_t len);
void printDecodedConfig(const UserConfiguration *config);

#endif