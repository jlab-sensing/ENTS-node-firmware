#ifndef CONFIG_SERVER_H
#define CONFIG_SERVER_H

#include <WebServer.h>
#include "configuration.h"

extern WebServer server;

void handleRoot();
void handleSave();
void setupServer();
void handleClient();

#endif