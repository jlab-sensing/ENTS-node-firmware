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

Dirtviz::Dirtviz(void) : url(nullptr) {}

Dirtviz::Dirtviz(const char *url, const uint16_t &port) {
  // set parameters
  this->SetUrl(url);
  this->SetPort(port);
}

Dirtviz::~Dirtviz() {
  // free memory
  free(this->url);
}

void Dirtviz::SetUrl(const char *new_url) {
  // get length of new url string, add 1 for null char
  size_t url_len = strlen(new_url);
  ++url_len;

  // allocate memory
  char *temp_url = reinterpret_cast<char *>(realloc(this->url, url_len));

  if (temp_url != nullptr) {
    this->url = temp_url;
    strncpy(
        this->url, new_url,
        url_len);  // strcpy is safe here because we just allocated enough space
  } else {
    // Handle allocation failure (e.g., set an error flag, use a default URL,
    // etc.)
  }
}

const char *Dirtviz::GetUrl(void) const { return this->url; }

void Dirtviz::SetPort(const uint16_t &new_port) { this->port = new_port; }

uint16_t Dirtviz::GetPort(void) const { return this->port; }

unsigned int Dirtviz::Check() const {
  Log.traceln("Dirtviz::Check");
  WiFiClient client;

  if (!client.connect(url, port)) {
    return 0;
  }

  Log.traceln("Sending GET request");

  // format request
  std::ostringstream req;
  req << "GET /api/ HTTP/1.1" << "\r\n";
  req << "Host: " << url << "\r\n";
  req << "User-Agent: curl/8.10.1" << "\r\n";
  req << "\r\n";

  // send full request to server
  int bytes_written = client.write(req.str().c_str());
  Log.traceln("Wrote %d bytes", bytes_written);

  Log.traceln("Done!");

  // wait until there's bytes available with timeout
  unsigned int timeout = millis() + g_resp_timeout;
  while (!client.available()) {
    // timeout
    if (millis() > timeout) {
      Log.noticeln("API Check timeout!");
      return (uint32_t)-1;
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

  // close connection
  client.flush();
  client.stop();

  // read string into an object
  HttpClient http_client(resp);

  unsigned int http_code = http_client.ResponseCode();
  if (http_code != 200) {
    Log.warningln("Api health check failed! Reponse code: %d", http_code);
  }

  return http_code;
}

HttpClient Dirtviz::SendMeasurement(const uint8_t *meas, size_t meas_len) {
  WiFiClient client;

  char buffer[100];

  Log.noticeln("WiFi status: %d", WiFi.status());

  // connect to server
  if (!client.connect(url, port)) {
    Log.errorln("Connection to %s:%d failed!", url, port);
    HttpClient empty;
    return empty;
  }

  // send data

  // format request
  // TODO(jmadden173) fix hardcoded api path
  std::ostringstream headers;
  headers << "POST /api/sensor/ HTTP/1.1" << "\r\n";
  headers << "Host: " << url << "\r\n";
  headers << "User-Agent: curl/8.10.1" << "\r\n";
  headers << "Content-Type: application/octet-stream" << "\r\n";
  headers << "Content-Length: " << meas_len << "\r\n";
  headers << "Connection: close" << "\r\n";
  headers << "\r\n";

  // copy stream to string
  std::string headers_str = headers.str();

  // copy data into request array
  std::vector<char> request;
  std::copy(headers_str.begin(), headers_str.end(),
            std::back_inserter(request));
  std::copy(meas, meas + meas_len, std::back_inserter(request));

  Log.traceln("Length of request: %d", request.size());
  int bytes_written = client.write(request.data(), request.size());
  Log.traceln("Wrote %d bytes", bytes_written);

  // read response

  // wait until there's bytes available with timeout
  unsigned int timeout = millis() + g_resp_timeout;
  while (!client.available()) {
    // timeout
    if (millis() > timeout) {
      Log.noticeln("Send measurement timeout!");
      Log.noticeln("WiFi status: %d", WiFi.status());
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

  // close connection
  client.flush();
  client.stop();

  // read string into an object
  HttpClient http_client(resp);

  return http_client;
}
