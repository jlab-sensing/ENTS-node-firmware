/**
 * @file test_main.cpp
 * @author John Madden (jmadden173@pm.me)
 * @brief Test HttpClient class
 * @version 0.1
 * @date 2024-11-01
 *
 * @copyright Copyright (c) 2024
 *
 */

#include <Arduino.h>
#include <unity.h>

#include "dirtviz.hpp"

void setUp(void) {}

void tearDown(void) {}

void TestHttpClient(void) {
  std::string resp =
      "HTTP/2 200\r\n"
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

  std::string resp_data =
      "{\r\n"
      "    \"hello\": \"I'm alive and healthy! Super healthy :D\"\r\n"
      "}";

  HttpClient client(resp);

  TEST_ASSERT_EQUAL_STRING("HTTP/2", client.Version().c_str());
  TEST_ASSERT_EQUAL(200, client.ResponseCode());

  TEST_ASSERT_EQUAL_STRING("application/json",
                           client.Header("content-type").c_str());
  TEST_ASSERT_EQUAL_STRING("59", client.Header("content-length").c_str());
  TEST_ASSERT_EQUAL_STRING("Fri, 01 Nov 2024 23:53:02 GMT",
                           client.Header("date").c_str());

  TEST_ASSERT_EQUAL_STRING(resp_data.c_str(), client.Data().c_str());
}

void setup() {
  // NOTE!!! Wait for >2 secs
  // if board doesn't support software reset via Serial.DTR/RTS
  delay(2000);

  UNITY_BEGIN();

  RUN_TEST(TestHttpClient);

  UNITY_END();
}

void loop() {}
