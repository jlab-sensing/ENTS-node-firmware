#ifndef CONFIG_SERVER_H
#define CONFIG_SERVER_H

#include <WebServer.h>
#include "configuration.hpp"
#include "modules/wifi_userconfig.hpp"

extern WebServer server;
extern ModuleHandler::ModuleUserConfig user_config;

void handleRoot();
void handleSave();
void setupServer();
void handleClient();

#endif
