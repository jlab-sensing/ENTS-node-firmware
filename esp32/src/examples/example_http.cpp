/**
 * @example example_http.cpp
 *
 * Parses a static HTTP request and prints the response code, headers, and
 * body.
 *
 * @author John Madden
 * @date 2024-11-06
 */

#include <Arduino.h>
#include <ArduinoLog.h>

#include "dirtviz.hpp"

void setup() {
  Serial.begin(115200);

  Log.begin(LOG_LEVEL_VERBOSE, &Serial);

  std::string resp =
      "HTTP/2 200 OK\r\n"
      "content-type: application/json\r\n"
      "content-length: 59\r\n"
      "date: Fri, 01 Nov 2024 23:53:02 GMT\r\n"
      "access-control-allow-origin: *\r\n"
      "server: gunicorn\r\n"
      "x-cache: Miss from cloudfront\r\n"
      "via: 1.1 4a628119373a320de50f1e6bc0e5f87a.cloudfront.net "
      "(CloudFront)\r\n"
      "x-amz-cf-pop: SFO5-P2\r\n"
      "x-amz-cf-id: "
      "t9EARJ8Hsu4FipZqiiVhsjwEU29sICEwjA7XMJNIOEXgOnJA-TlhdA==\r\n"
      "\r\n"
      "{\r\n"
      "    \"hello\": \"I'm alive and healthy! Super healthy :D\"\r\n"
      "}\r\n";

  Log.traceln("Input string:\r\n%s", resp.c_str());

  HttpClient client(resp);

  Log.traceln("Version: %s", client.Version().c_str());
  Log.traceln("Return code: %d", client.ResponseCode());
  Log.traceln("date: %s", client.Header("date").c_str());
  Log.traceln("data:\r\n%s", client.Data().c_str());
}

void loop() {}
