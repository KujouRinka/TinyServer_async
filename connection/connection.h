#ifndef TINYSERVER_ASYNC_CONNECTION_H
#define TINYSERVER_ASYNC_CONNECTION_H

#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <algorithm>

#include <asio.hpp>

using namespace std;
using namespace asio;

using Buffer = asio::streambuf;

class State;
class ParseReqTest;
class MockConnection;
class ComposeBuffer;
class Responding;

struct Request {
  string method;
  string path;
  string version;
  unordered_map<string, string> headers;
  string body;
};

struct Response {
  string version;
  string status;
  string reason;
  unordered_map<string, string> headers;
  string body;
};

constexpr size_t BUFFER_SIZE = 2048;

/**
 * Connection class maintains all the state of the connection.
 * It defines member functions to handle the all.
 */
class Connection : public enable_shared_from_this<Connection> {
  friend class ParseReqTest;
  friend class Responding;
 public:
  explicit Connection(ip::tcp::socket socket);
  ~Connection();
  virtual void inRead();
  virtual void inWrite();
  virtual void prepareResp(shared_ptr<Connection> holder);

  Buffer &read_buf();
  Buffer &write_buf();

 public:
  size_t rSize();
  size_t rCap();
  size_t rFree();
  size_t wSize();
  size_t wCap();
  size_t wFree();

  void assignTask();

 private:
  ip::tcp::socket _remote;
  Buffer _read_buf;
  Buffer _write_buf;
  // mark is server good
  bool _good;
  string _bad_code;
  // infinite state machine
  shared_ptr<State> _state;
  shared_ptr<ComposeBuffer> _resp_buf;

  // request info
 private:
  Request _request;

 public:
  void setBad(string bad_code);
  void setReqMethod(const string &s);
  void setReqPath(string s);
  void setReqVersion(string s);
  string getReqHeader(string key);
  void setReqHeader(string key, string value);
  void setReqBody(string body);

  // request info
 private:
  Response _response;
  static unordered_map<string, string> _status_map;

  void setStatus(string status_code);

 public:
  void setRespHeader(string key, string value);

 private:
  void GetHandler();
  void PostHandler();
  // TODO: add more handler
  void BadHandler();
};

inline Buffer &Connection::read_buf() {
  return _read_buf;
}

inline Buffer &Connection::write_buf() {
  return _write_buf;
}

inline size_t Connection::rSize() {
  return _read_buf.size();
}

inline size_t Connection::rCap() {
  return _read_buf.max_size();
}

inline size_t Connection::rFree() {
  return rCap() - rSize();
}

inline size_t Connection::wSize() {
  return _write_buf.size();
}

inline size_t Connection::wCap() {
  return _write_buf.max_size();
}

inline size_t Connection::wFree() {
  return wCap() - wSize();
}

inline void Connection::setBad(string bad_code) {
  _good = false;
  _bad_code = std::move(bad_code);
}

inline void Connection::setReqMethod(const string &s) {
  if (strcasecmp(s.c_str(), "GET") == 0)
    _request.method = "GET";
  else if (strcasecmp(s.c_str(), "POST") == 0)
    _request.method = "POST";
  else if (strcasecmp(s.c_str(), "HEAD") == 0)
    _request.method = "HEAD";
  else if (strcasecmp(s.c_str(), "PUT") == 0)
    _request.method = "PUT";
  else if (strcasecmp(s.c_str(), "DELETE") == 0)
    _request.method = "DELETE";
  else if (strcasecmp(s.c_str(), "CONNECT") == 0)
    _request.method = "CONNECT";
  else if (strcasecmp(s.c_str(), "OPTIONS") == 0)
    _request.method = "OPTIONS";
  else if (strcasecmp(s.c_str(), "TRACE") == 0)
    _request.method = "TRACE";
  else if (strcasecmp(s.c_str(), "PATCH") == 0)
    _request.method = "PATCH";
  else
    _request.method = "UNKNOWN";
}

inline void Connection::setReqPath(string s) {
  _request.path = std::move(s);
}

inline void Connection::setReqVersion(string s) {
  _request.version = std::move(s);
}

inline string Connection::getReqHeader(string key) {
  for_each(key.begin(), key.end(), [](char &c) {
    c = tolower(c);
  });
  return _request.headers[std::move(key)];
}

inline void Connection::setReqHeader(string key, string value) {
  for_each(key.begin(), key.end(), [](char &c) {
    c = tolower(c);
  });
  _request.headers.insert(make_pair(std::move(key), std::move(value)));
}

inline void Connection::setReqBody(string body) {
  _request.body = std::move(body);
}

inline void Connection::setStatus(string status_code) {
  _response.reason = _status_map[status_code];
  _response.status = std::move(status_code);
}

inline void Connection::setRespHeader(string key, string value) {
  _response.headers.insert(make_pair(std::move(key), std::move(value)));
}

#endif //TINYSERVER_ASYNC_CONNECTION_H
