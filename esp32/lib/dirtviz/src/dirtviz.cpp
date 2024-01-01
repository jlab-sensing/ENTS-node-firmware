/**
 * @see dirtviz.hpp
 * 
 * @author John Madden <jmadden173@pm.me>
 * @date 2023-11-29
*/

#include "dirtviz.hpp"

Dirtviz::Dirtviz(const std::string& url, const uint16_t& port)
{
  // set parameters
  this->SetUrl(url);
  this->SetPort(port);
  
}

void Dirtviz::SetUrl(const std::string& new_url)
{
  this->url = new_url;
}

std::string Dirtviz::GetUrl(void) const
{
  return this->url;
}

void Dirtviz::SetPort(const uint16_t &new_port)
{
  this->port = new_port;
}

uint16_t Dirtviz::GetPort(void) const
{
  return this->port;
}

size_t Dirtviz::SendMeasurement(const uint8_t *meas, size_t meas_len,
                                uint8_t *resp)
{
  // WiFi client for connection with API
  WiFiClient client;

  // try connection return negative length if error
  if (!client.connect(this->url, this->port))
  {
    return -1;
  }

  // send data

  // HTTP command, path, and version
  client.println("POST [PATH] HTTP/1.1");
  // who we are posting to
  client.println("Host: [HOST]");
  // type of data
  client.println("Content-Type: application/octet-stream");
  // length of data (specific to application/octet-stream)
  client.println("Content-Length: [LENGTH]");
  // close connection after data is sent
  client.println("Connection: close");
  // newline indicating end of headers
  client.println();
  // send data
  client.print((char *) meas);


  // read response
  
  // TODO implement with malloc to store in dynamic buffer with separate call
  // that gets the response. That way the HTTP code is handled on the ESP32
  // and "hopefully" the binary data that gets sent back is passed directly
  // over I2C

  // create buffer
  int resp_len = client.available();
  char resp_buf[resp_len];

  for (int idx = 0; idx < resp_len; ++idx)
  {
    resp_buf[idx] = client.read();
  }

  // disconnect after message is sent
  client.stop();
}