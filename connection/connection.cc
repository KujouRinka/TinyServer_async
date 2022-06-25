#include "connection.h"

#include <fstream>
#include <utility>
#include <unistd.h>
#include <sys/stat.h>

#include "state.h"
#include "common/common.h"
#include "common/buffer.h"

Connection::Connection(ip::tcp::socket socket)
  : _state(new ParseReq(this)), _remote(std::move(socket)),
    _read_buf(BUFFER_SIZE), _write_buf(BUFFER_SIZE) {}

Connection::~Connection() {
  if (_remote.is_open())
    _remote.close();
}

void Connection::inRead() {
  _remote.async_read_some(
    _read_buf.prepare(rFree()),
    [this, cap = shared_from_this()](const error_code &err, ssize_t sz) {
      if (err && err != error::eof) {
        // close conn
        return;
      }
      // TODO: all thing pass to thread pool
      //  by calling assignTask()
      _read_buf.commit(sz);
      _state->go(cap);
    }
  );
}

void Connection::inWrite() {
  _remote.async_write_some(
    _write_buf.data(),
    [this, cap = shared_from_this()](const error_code &err, ssize_t sz) {
      if (err && err != error::eof) {
        // close conn
        return;
      }
      // TODO: all thing pass to thread pool
      //  by calling assignTask()
      _write_buf.consume(sz);
      _state->go(cap);
    }
  );
}

void Connection::prepareResp(shared_ptr<Connection> holder) {
  if (_request.method == "GET") {
    GetHandler();
  } else if (_request.method == "POST") {
    PostHandler();
  } else {
    // TODO: bad request
  }
  _state = make_shared<Responding>(this);
  _state->go(std::move(holder));
}

void Connection::assignTask() {
}

void Connection::GetHandler() {
  // redirect for main page
  _response.version = "HTTP/1.1";
  if (_request.path == "/") {
    setStatus("302");
    setRespHeader("Location", "/index.html");
    _resp_buf.reset(new ComposeBuffer{new ResponseBlockBuffer(_response)});
    return;
  }
  if (!isFile(_request.path)) {
    setStatus("404");
    _resp_buf.reset(new ComposeBuffer{new ResponseBlockBuffer(_response)});
    return;
  }
  ifstream file(_request.path, ios::in | ios::binary);
  if (!file.is_open()) {
    setStatus("404");
    _resp_buf.reset(new ComposeBuffer{new ResponseBlockBuffer(_response)});
    return;
  }
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
}

void Connection::PostHandler() {
  // TODO:
}
