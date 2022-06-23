#include "connection.h"

#include <unistd.h>
#include <sys/stat.h>

#include "state.h"
#include "common/buffer.h"

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
    _response.version = "HTTP/1.1";
    ifstream file(_request.path, ios::in | ios::binary);
    if (!file.is_open())
        setStatus("404");
    else
        setStatus("200");
    // int file = open(_request.path.c_str(), O_RDONLY);
    // struct stat f_stat;
    // fstat(file, &f_stat);
    // TODO:
    file.seekg(0, ios::end);
    setRespHeader("Content-Length", to_string(file.tellg()));
    file.seekg(0, ios::beg);

    BlockBuffer *info_buf = new ResponseBlockBuffer(_response);
    BlockBuffer *body_buf = new FileIterBlockBuffer(std::move(file));
    auto *compose_buf = new ComposeBuffer({info_buf, body_buf});
    _resp_buf.reset(compose_buf);

    _state = make_shared<Responding>(this);
    _state->go();
}

void Connection::assignTask() {
}
