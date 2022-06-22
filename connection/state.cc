#include "state.h"

#include "connection.h"

// ---------------------------------------------
// State definitions
State::State(Connection *conn)
    : _conn(conn) {}

// ---------------------------------------------
// ParseReq definitions

std::regex ParseReq::req_re("^(GET|POST)\\s([^\\s]+)\\s(HTTP\\/1\\.1)$\\r\\n", std::regex::icase);
std::regex ParseReq::headers_re("^\\s*(.+?)\\s*:\\s*(.+?)\\s*?\\r\\n$");
ParseReq::ParseReq(Connection *conn)
    : State(conn), step(Req_line),
      line_begin(-1), line_end(0), content_length(0), next_begin(0) {}

void ParseReq::go() {
    // read all data
    req_buffer.append(
        buffer_cast<const char *>(_conn->read_buf().data()),
        _conn->rSize()
    );
    _conn->read_buf().consume(_conn->rSize());
    if (step == Body) {
        body();
    } else {
        while (nextLine()) {
            if (step == Req_line) {
                req();
            } else if (step == Headers) {
                headers();
            }
        }
    }

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
        _conn->setMethod(matcher[1]);
        _conn->setPath(matcher[2]);
        // _conn->setVersion(matcher[3]);
        step = Headers;
    } else {
        // bad
    }
}

void ParseReq::headers() {
    if (line_end - line_begin == 2 &&
        req_buffer[line_begin] == '\r' && req_buffer[line_end] == '\n') {

        // set Content-Length
        if (_conn->getHeader("Content-Length") != "") {
            try {
                content_length = stoi(_conn->getHeader("Content-Length"), nullptr);
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
        req_buffer.cend() + line_end,
        matcher, headers_re)) {
        _conn->setHeader(matcher[1], matcher[2]);
    }
}

void ParseReq::body() {
    if (int len = req_buffer.size() - line_end; len < content_length) {
        _conn->inRead();
    } else if (len == content_length) {
        _conn->setBody(string(req_buffer.begin() + line_end, req_buffer.end()));
        step = OK;
        _conn->prepareResp();
    } else {
        // bad
    }
}

// ---------------------------------------------
// ParseRead definitions

ParseRead::ParseRead(Connection *conn)
    : State(conn) {}

void ParseRead::go() {

}

// ---------------------------------------------
// ParseWrite definitions

ParseWrite::ParseWrite(Connection *conn)
    : State(conn) {}

void ParseWrite::go() {

}

// ---------------------------------------------
// Bad definitions

Bad::Bad(Connection *conn)
    : State(conn) {}

void Bad::go() {

}
