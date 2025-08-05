#ifndef VALIDATION_H
#define VALIDATION_H

#include <WebServer.h>
#include "configuration.h"

extern WebServer server;

String validateInputs();
String validateUInt(const String& value, const String& name);
String validateFloat(const String& value, const String& name);
String validateURL(const String& value);

#endif