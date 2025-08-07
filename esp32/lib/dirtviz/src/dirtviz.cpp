/**
 * @see dirtviz.hpp
 *
 * @author John Madden <jmadden173@pm.me>
 * @date 2023-11-29
 */

#include "dirtviz.hpp"

#include <Arduino.h>
#include <ArduinoLog.h>

#include <iomanip>
#include <sstream>
#include <vector>

/** Timeout for http responses */
unsigned int g_resp_timeout = 1000;

/** Max size of HTTP POST request */
const size_t g_request_size = 512;

// TODO(jmadden173): update to dynamic version
/** User agent string */
const char g_user_agent[] = "ents/2.3.0";

Dirtviz::Dirtviz(void) {}

Dirtviz::Dirtviz(const char *url) { SetUrl(url); }

Dirtviz::~Dirtviz() {}

void Dirtviz::SetUrl(const char *url) {
  Log.traceln("Dirtviz::SetUrl");
  Log.traceln("Setting URL to %s", url);
  this->url.setUrl(url);
  Log.traceln("URL stored is %s", this->url.getUrl().c_str());
}

unsigned int Dirtviz::Check() {
  Log.traceln("Dirtviz::Check");
  Log.noticeln("Connecting to %s:%d", url.getHost().c_str(), url.getPort());

  if (!client.connect(url.getHost().c_str(), (uint16_t)url.getPort())) {
    return 0;
  }

  Log.traceln("Sending GET request");

  // format request
  std::ostringstream req;
  req << "GET " << "/api/" << " HTTP/1.1" << "\r\n";
  req << "Host: " << url.getHost().c_str() << "\r\n";
  req << "User-Agent: " << g_user_agent << "\r\n";
  req << "\r\n";

  // send full request to server
  int bytes_written = client.write(req.str().c_str());
  Log.noticeln("Wrote %d bytes to server", bytes_written);

  return bytes_written;
}

unsigned int Dirtviz::SendMeasurement(const uint8_t *meas, size_t meas_len) {
  char buffer[100];

  Log.noticeln("Sending measurement");
  Log.traceln("WiFi status: %d", WiFi.status());

  // connect to server
  if (!client.connect(url.getHost().c_str(), (uint16_t)url.getPort())) {
    Log.errorln("Connection to %s:%d failed!", url.getHost(), url.getPort());
  }

  // send data

  // format request
  std::ostringstream headers;
  headers << "POST " << "/" << url.getPath().c_str() << " HTTP/1.1" << "\r\n";
  headers << "Host: " << url.getHost().c_str() << "\r\n";
  headers << "User-Agent: " << g_user_agent << "\r\n";
  headers << "Content-Type: application/octet-stream" << "\r\n";
  headers << "Content-Length: " << meas_len << "\r\n";
  headers << "Connection: close" << "\r\n";
  headers << "\r\n";

  // copy stream to string
  std::string headers_str = headers.str();

  Log.verboseln("Headers:\r\n----\r\n%s\r\n----\r\n", headers_str.c_str());

  // copy data into request array
  std::vector<char> request;
  std::copy(headers_str.begin(), headers_str.end(),
            std::back_inserter(request));
  std::copy(meas, meas + meas_len, std::back_inserter(request));

  Log.traceln("Length of request: %d", request.size());
  int bytes_written = client.write(request.data(), request.size());
  Log.noticeln("Wrote %d bytes to server", bytes_written);

  return bytes_written;
}

HttpClient Dirtviz::GetResponse() {
  // wait until there's bytes available with timeout
  unsigned int timeout = millis() + g_resp_timeout;
  while (!client.available()) {
    // timeout
    if (millis() > timeout) {
      Log.warningln("Response timeout!");
      Log.warningln("WiFi status: %d", WiFi.status());
      HttpClient empty_resp;

      client.flush();
      client.stop();

      return empty_resp;
    }

    // Log.traceln("Delaying until available");
    delay(100);
  }

  // read response
  std::string resp;
  while (client.available()) {
    char c = client.read();
    resp += c;
  }
  Log.warningln("%s",resp.c_str());
  // close connection
  client.flush();
  client.stop();

  // read string into an object
  HttpClient http_client(resp);

  return http_client;
}
