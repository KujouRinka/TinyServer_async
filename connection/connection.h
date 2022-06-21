#ifndef TINYSERVER_ASYNC_CONNECTION_H
#define TINYSERVER_ASYNC_CONNECTION_H

#include <asio.hpp>

using namespace asio;

using Buffer = streambuf;

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
    Connection(ip::tcp::socket socket);
    ~Connection();
    void inRead();
    void inWrite();

private:
    ConnStat _stat;
    ip::tcp::socket _remote;

    Buffer _read_buf;
    Buffer _write_buf;

private:
    size_t rSize();
    size_t rCap();
    size_t rFree();
    size_t wSize();
    size_t wCap();
    size_t wFree();
};

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

#endif //TINYSERVER_ASYNC_CONNECTION_H
