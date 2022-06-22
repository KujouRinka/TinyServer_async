#include "connection.h"

#include <fstream>

#include "common/common.h"
#include "state.h"

Connection::Connection(ip::tcp::socket socket)
    : _state(new ParseReq(this)), _remote(std::move(socket)),
      _read_buf(BUFFER_SIZE), _write_buf(BUFFER_SIZE) {}

Connection::~Connection() {}

void Connection::inRead() {
    _remote.async_read_some(
        _read_buf.prepare(rFree()),
        [this](const error_code &err, ssize_t sz) {
            if (err && err != error::eof) {
                // close conn
            }
            // TODO: all thing pass to thread pool
            _read_buf.commit(sz);
            _state->go();
        }
    );
}

void Connection::inWrite() {
    _remote.async_write_some(
        _write_buf.data(),
        [this](const error_code &err, ssize_t sz) {
            if (err && err != error::eof) {
                // close conn
            }
            // TODO: all thing pass to thread pool
            _write_buf.consume(sz);
            _state->go();
        }
    );
}

void Connection::prepareResp() {
}

void Connection::assignTask() {
}
