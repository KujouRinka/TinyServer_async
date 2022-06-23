#include "state.h"

#include "connection.h"
#include "common/buffer.h"

// ---------------------------------------------
// State definitions
State::State(Connection *conn)
    : _conn(conn) {}

// ---------------------------------------------
// ParseReq definitions

std::regex ParseReq::req_re("^(GET|POST)\\s([^\\s]+)\\s(HTTP\\/1\\.1)\\r\\n$", std::regex::icase);
std::regex ParseReq::headers_re("^\\s*(.+?)\\s*:\\s*(.+?)\\s*?\\r\\n$");
ParseReq::ParseReq(Connection *conn)
    : State(conn), step(Req_line),
      line_begin(-1), line_end(0), content_length(0), next_begin(0) {}

// TODO: implement BAD handler
void ParseReq::go() {
    // read all data
    req_buffer.append(
        buffer_cast<const char *>(_conn->read_buf().data()),
        _conn->rSize()
    );
    _conn->read_buf().consume(_conn->rSize());
    while (line_end < req_buffer.size() && nextLine()) {
        if (step == Req_line) {
            req();
        } else if (step == Headers) {
            headers();
        }
    }
    if (step == Body)
        body();

    if (step != OK)
        _conn->inRead();
}

bool ParseReq::nextLine() {
    line_begin = next_begin;
    auto sz = req_buffer.size();
    while (line_end < sz) {
        if (req_buffer[line_end] == '\n' && req_buffer[line_end - 1] == '\r') {
            // line_begin = next_begin;
            ++line_end;
            next_begin = line_end;
            return true;
        }
        ++line_end;
    }
    return false;
}

void ParseReq::req() {
    std::smatch matcher;
    if (std::regex_match(
        req_buffer.cbegin() + line_begin,
        req_buffer.cbegin() + line_end,
        matcher, req_re)) {
        _conn->setReqMethod(matcher[1]);
        _conn->setReqPath(matcher[2]);
        // _conn->setVersion(matcher[3]);
        step = Headers;
    } else {
        // bad
    }
}

void ParseReq::headers() {
    if (line_end - line_begin == 2 &&
        req_buffer[line_begin] == '\r' && req_buffer[line_begin + 1] == '\n') {

        // set Content-Length
        if (!_conn->getReqHeader("Content-Length").empty()) {
            try {
                content_length = stoi(_conn->getReqHeader("Content-Length"), nullptr);
            } catch (const exception &e) {
                // bad
            }
        }
        step = Body;
        return;
    }
    std::smatch matcher;
    if (std::regex_match(
        req_buffer.cbegin() + line_begin,
        req_buffer.cbegin() + line_end,
        matcher, headers_re)) {
        _conn->setReqHeader(matcher[1], matcher[2]);
    }
}

void ParseReq::body() {
    if (int len = req_buffer.size() - line_begin; len < content_length) {
        _conn->inRead();
    } else if (len == content_length) {
        _conn->setReqBody(string(req_buffer.begin() + line_begin, req_buffer.end()));
        step = OK;
        _conn->prepareResp();
    } else {
        // bad
    }
}

// ---------------------------------------------
// Writing definitions

Responding::Responding(Connection *conn)
    : State(conn), os(&_conn->_write_buf) {}

void Responding::go() {
    auto read_n = _conn->wFree();
    if (read_n > 0) {
        string data = _conn->_resp_buf->get(read_n);
        if (data.empty()) {
            // _conn->_state = make_shared<Closing>(_conn);
            // TODO: close connection
            return;
        }
        os << data;
    }
    _conn->inWrite();
}

// ---------------------------------------------
// Bad definitions

Bad::Bad(Connection *conn)
    : State(conn) {}

void Bad::go() {

}
