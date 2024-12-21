/**
 * @see dirtviz.hpp
 *
 * @author John Madden <jmadden173@pm.me>
 * @date 2023-11-29
 */

#include "dirtviz.hpp"
#include <Arduino.h>

#include <cstdio>

Dirtviz::Dirtviz(const char *url, const uint16_t &port)
    : url(nullptr), response(nullptr) {
  // set parameters
  this->SetUrl(url);
  this->SetPort(port);
}

Dirtviz::~Dirtviz() {
  // free memory
  free(this->url);
  free(this->response);
}

void Dirtviz::SetUrl(const char *new_url) {
  // get length of new url string, add 1 for null char
  size_t url_len = strlen(new_url);
  ++url_len;

  // allocate memory
  char *temp_url = reinterpret_cast<char *>(realloc(this->url, url_len));

  if (temp_url != nullptr) {
    this->url = temp_url;
    // strcpy is safe here because we just allocated enough space
    strncpy(this->url, new_url, url_len);
  } else {
    // Handle allocation failure (e.g., set an error flag, use a default URL,
    // etc.)
  }
}

const char *Dirtviz::GetUrl(void) const { return this->url; }

void Dirtviz::SetPort(const uint16_t &new_port) { this->port = new_port; }

uint16_t Dirtviz::GetPort(void) const { return this->port; }

int Dirtviz::SendMeasurement(const uint8_t *meas, size_t meas_len) {
  // WiFi client for connection with API
  // WiFiClientSecure client;
  WiFiClient client;

  // buffer for making post requests
  char buffer[100];
  Serial.print(this->url);
  Serial.print(":");
  Serial.print(this->port);
  // try connection return negative length if error
  // client.setInsecure();
  // client.setCACert(rootCACertificate);
  if (!client.connect(this->url, this->port)) {
    Serial.println("Connection failure");
    return -1;
  }

  // send data

  // HTTP command, path, and version
  client.println("POST / HTTP/1.1");
  // who we are posting to
  client.print("Host: ");
  client.print(this->url);
  client.print(":");
  client.println(this->port);
  // type of data
  client.println("Content-Type: application/octet-stream");
  // length of data (specific to application/octet-stream)
  snprintf(buffer, sizeof(buffer), "Content-Length: %d", meas_len);
  client.println(buffer);
  // close connection after data is sent
  client.println("Connection: close");
  // newline indicating end of headers
  client.println();
  // send data
  for (int idx = 0; idx < meas_len; ++idx) {
    client.write(meas[idx]);
  }

  // read response

  // get length of response
  int resp_len = client.available();

  // free memory before allocating to prevent leaks
  free(this->response);
  this->response = nullptr;

  // allocate memory
  this->response =
      reinterpret_cast<char *>(realloc(this->response, resp_len + 1));

  // copy into buffer
  if (this->response != nullptr) {
    // Ensure to read only available bytes and null-terminate the response
    int bytesRead = 0;
    while (client.available() && bytesRead < resp_len) {
      this->response[bytesRead++] = client.read();
    }
    this->response[bytesRead] = '\0';  // Null-terminate the response
  } else {
    Serial.println("Null pointer failure");
    return -1;
  }
  Serial.println(this->response);

  // disconnect after message is sent
  client.stop();

  // find status code
  int status_code;
  if (sscanf(this->response, "%*s %d", &status_code) != 1) {
    Serial.println("Unable to parse status code");
    return -1;
  }

  return status_code;
}

size_t Dirtviz::GetResponse(const uint8_t *data) const {
  // find response length from header

  // get pointer to start of line
  const char *length_start = strstr(this->response, "Content-Length:");
  if (length_start == nullptr) {
    return 0;
  }

  // parse the length
  size_t data_len;
  if (sscanf(length_start, "%*s %u", &data_len)) {
    return 0;
  }

  // read binary data, look for double CRLF
  data = (const uint8_t *)strstr(this->response, "\r\n\r\n");
  data += 4;

  // return the length of data
  return data_len;
}
