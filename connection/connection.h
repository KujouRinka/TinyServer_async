#ifndef TINYSERVER_ASYNC_CONNECTION_H
#define TINYSERVER_ASYNC_CONNECTION_H

#include <memory>
#include <string>
#include <unordered_map>

#include <asio.hpp>

using namespace std;
using namespace asio;

using Buffer = asio::streambuf;
class State;

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
public:
    explicit Connection(ip::tcp::socket socket);
    ~Connection();
    void inRead();
    void inWrite();
    void prepareResp();

    Buffer &read_buf();
    Buffer &write_buf();

public:
    size_t rSize();
    size_t rCap();
    size_t rFree();
    size_t wSize();
    size_t wCap();
    size_t wFree();

    void setMethod(string s);
    void setPath(string s);
    void setVersion(string s);
    string getHeader(string key);
    void setHeader(string key, string value);
    void setBody(string body);

private:
    ip::tcp::socket _remote;

    Buffer _read_buf;
    Buffer _write_buf;
    unique_ptr<State> _state;

    unordered_map<string, string> _headers;
private:
    void assignTask();

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

inline void Connection::setMethod(string s) {
    // TODO:
}

inline void Connection::setPath(string s) {
    // TODO:
}

inline void Connection::setVersion(string s) {
    // TODO:
}

inline string Connection::getHeader(string key) {
    return "";
}

inline void Connection::setHeader(string key, string value) {
    // TODO:
}

inline void Connection::setBody(string body) {
    // TODO:
}

#endif //TINYSERVER_ASYNC_CONNECTION_H
