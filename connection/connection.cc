#include "connection.h"
#include "common.h"

Connection::Connection(ip::tcp::socket socket)
    : _stat(ParsingReq), _remote(std::move(socket)),
      _read_buf(BUFFER_SIZE), _write_buf(BUFFER_SIZE) {}

Connection::~Connection() {}

void Connection::inRead() {
    _remote.async_read_some(
        _read_buf.prepare(rFree()),
        [this](const error_code &err, ssize_t sz) {
            _read_buf.commit(sz);
            // TODO:
            // pass task to thread pool
        }
    );
}

void Connection::inWrite() {
    _remote.async_write_some(
        _write_buf.data(),
        [this](const error_code &err, ssize_t sz) {
            _write_buf.consume(sz);
            // TODO:
            // do something
        }
    );
}
