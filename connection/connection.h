#ifndef TINYSERVER_ASYNC_CONNECTION_H
#define TINYSERVER_ASYNC_CONNECTION_H

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

enum ConnStat {
    ParsingReq = 0,
    ShouldRead,
    ShouldWrite,
    Bad,
};

constexpr size_t BUFFER_SIZE = 2048;

/**
 * Connection class maintains all the state of the connection.
 * It defines member functions to handle the all.
 */
class Connection {
    friend class ParseReqTest;
public:
    explicit Connection(ip::tcp::socket socket);
    ~Connection();
    virtual void inRead();
    virtual void inWrite();
    virtual void prepareResp();

    Buffer &read_buf();
    Buffer &write_buf();

public:
    size_t rSize();
    size_t rCap();
    size_t rFree();
    size_t wSize();
    size_t wCap();
    size_t wFree();

    void setMethod(const string &s);
    void setPath(string s);
    void setVersion(string s);
    string getHeader(string key);
    void setHeader(string key, string value);
    void setBody(string body);

    void assignTask();

private:
    ip::tcp::socket _remote;

    Buffer _read_buf;
    Buffer _write_buf;
    shared_ptr<State> _state;

private:
    struct Request {
        string method;
        string path;
        string version;
        unordered_map<string, string> headers;
        string body;
    };
    Request _request;

private:
    struct Response {
        string version;
        string status;
        string reason;
        unordered_map<string, string> headers;
        string body;
    };
    Response _response;
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

inline void Connection::setMethod(const string &s) {
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

inline void Connection::setPath(string s) {
    _request.path = std::move(s);
}

inline void Connection::setVersion(string s) {
    // TODO:
}

inline string Connection::getHeader(string key) {
    for_each(key.begin(), key.end(), [](char &c) {
        c = tolower(c);
    });
    return _request.headers[std::move(key)];
}

inline void Connection::setHeader(string key, string value) {
    for_each(key.begin(), key.end(), [](char &c) {
        c = tolower(c);
    });
    _request.headers.insert(make_pair(std::move(key), std::move(value)));
}

inline void Connection::setBody(string body) {
    _request.body = std::move(body);
}

#endif //TINYSERVER_ASYNC_CONNECTION_H
