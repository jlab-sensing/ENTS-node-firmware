#include "http.hpp"

#include <Arduino.h>

#include <algorithm>
#include <sstream>

HttpClient::HttpClient() {}

HttpClient::HttpClient(const std::string &resp) {
  // find indicies of data
  size_t header_start = resp.find(LINE_END);
  size_t data_start = resp.find(LINE_END + LINE_END);
  size_t header_end = data_start - header_start;
  std::string status_substr = resp.substr(0, header_start);
  ExtractText(&status_substr);
  DecodeStatus(status_substr);

  std::string headers_substr = resp.substr(header_start, header_end);
  ExtractText(&headers_substr);
  DecodeHeaders(headers_substr);

  data = resp.substr(data_start);
  ExtractText(&data);
}

HttpClient::~HttpClient() {}

std::string HttpClient::Version() { return version; }

unsigned int HttpClient::ResponseCode() { return code; }

std::string HttpClient::Header(std::string header) { return headers[header]; }

std::string HttpClient::Data() { return data; }

void HttpClient::DecodeStatus(std::string status_str) {
  std::stringstream status_stream(status_str);
  status_stream >> version >> code;
}

void HttpClient::DecodeHeaders(std::string headers_str) {
  std::istringstream header_stream(headers_str);

  for (std::string line; std::getline(header_stream, line, '\n');) {
    std::size_t pos = line.find(':');
    std::string key = line.substr(0, pos);
    ExtractText(&key);
    std::string val = line.substr(pos + 1);
    ExtractText(&val);
    headers[key] = val;
  }
}

void HttpClient::RemoveLeadingSpace(std::string *str) {
  // get first printable character
  auto end = std::find_if(str->begin(), str->end(),
                          [](char c) { return !isSpace(c); });

  if (str->begin() != end) {
    // remove whitespace
    str->erase(
        std::remove_if(str->begin(), end, [](char c) { return isSpace(c); }),
        end);
  }
}

void HttpClient::RemoveTrailingSpace(std::string *str) {
  // reverse string
  std::reverse(str->begin(), str->end());

  // get iterator to first non-space character
  auto rend = std::find_if(str->begin(), str->end(),
                           [](char c) { return !isSpace(c); });

  // no whitespace found
  if (str->begin() != rend) {
    // remove whitespace
    str->erase(
        std::remove_if(str->begin(), rend, [](char c) { return isSpace(c); }),
        rend);
  }

  // restore to original order
  std::reverse(str->begin(), str->end());
}

void HttpClient::ExtractText(std::string *str) {
  RemoveLeadingSpace(str);
  RemoveTrailingSpace(str);
}

void HttpClient::RemoveSpace(std::string *str) {
  // remove whitespace
  str->erase(std::remove_if(str->begin(), str->end(),
                            [](char c) { return isSpace(c); }),
             str->end());
}
