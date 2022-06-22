#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <string>

#include "connection/connection.h"
#include "connection/state.h"

using namespace std;

class ParseLineTest : public ::testing::Test {
protected:
    void SetUp() override {
        req = new ParseReq(nullptr);
    }

    void TearDown() override {
        delete req;
    }

    void reset() {
        TearDown();
        SetUp();
    }

    string &buffer() {
        return req->req_buffer;
    }

    int &line_begin() {
        return req->line_begin;
    }

    int &line_end() {
        return req->line_end;
    }

    int &next_begin() {
        return req->next_begin;
    }

    bool nextLine() {
        return req->nextLine();
    }

    ParseReq *req;
};

TEST_F(ParseLineTest, parseLineTest) {
    string req_line = "GET /helloworld HTTP/1.1\r\n";
    string headers = "k1:v1\r\nk2 : v2\r\nk3: v3\r\n";
    buffer() = req_line + headers;
    // first line
    EXPECT_EQ(nextLine(), true);
    EXPECT_EQ(line_begin(), 0);
    EXPECT_EQ(line_end(), req_line.size());
    // second line
    EXPECT_EQ(nextLine(), true);
    EXPECT_EQ(line_begin(), req_line.size());
    EXPECT_EQ(line_end(), req_line.size() + 7);

    reset();
    string partial_req_line1 = "GET /hello";
    string partial_req_line2 = "world HTT";
    string partial_req_line3 = "P/1.1\r\n";
    buffer() = partial_req_line1;
    // partial test
    EXPECT_EQ(nextLine(), false);
    EXPECT_EQ(line_begin(), 0);
    EXPECT_EQ(line_end(), partial_req_line1.size());
    // retest
    EXPECT_EQ(nextLine(), false);
    EXPECT_EQ(line_begin(), 0);
    EXPECT_EQ(line_end(), partial_req_line1.size());
    // append test
    buffer() += partial_req_line2;
    EXPECT_EQ(nextLine(), false);
    EXPECT_EQ(line_begin(), 0);
    EXPECT_EQ(line_end(), partial_req_line1.size() + partial_req_line2.size());
    // full line test
    buffer() += partial_req_line3;
    EXPECT_EQ(nextLine(), true);
    EXPECT_EQ(line_begin(), 0);
    EXPECT_EQ(line_end(), partial_req_line1.size() + partial_req_line2.size() + partial_req_line3.size());
}

class MockConnection : public Connection {
public:
    MockConnection(ip::tcp::socket socket) : Connection(std::move(socket)) {}
    MOCK_METHOD(void, inRead, (), (override));
    MOCK_METHOD(void, inWrite, (), (override));
    MOCK_METHOD(void, prepareResp, (), (override));
};

class ParseReqTest : public ::testing::Test {
protected:
    void SetUp() override {
        io_context ctx;
        conn = new MockConnection(ip::tcp::socket(ctx));
        req = new ParseReq(conn);
    }

    void TearDown() override {
        delete req;
        delete conn;
    }

    void reset() {
        TearDown();
        SetUp();
    }

    Buffer &read_buf() {
        return conn->read_buf();
    }

    Buffer &write_buf() {
        return conn->write_buf();
    }

    shared_ptr<State> state() {
        return conn->_state;
    }

    string &header(string key) {
        return conn->_headers[key];
    }

    Connection::METHOD method() {
        return conn->_method;
    }

    string &path() {
        return conn->_path;
    }

    string &body() {
        return conn->_body;
    }

    string &buffer() {
        return req->req_buffer;
    }

    int &line_begin() {
        return req->line_begin;
    }

    int &line_end() {
        return req->line_end;
    }

    int &next_begin() {
        return req->next_begin;
    }

    bool nextLine() {
        return req->nextLine();
    }

    ParseReq::STEP step() {
        return req->step;
    }

    MockConnection *conn;
    ParseReq *req;
};

TEST_F(ParseReqTest, parseReqTest) {
    // with request line
    iostream read_stream(&read_buf());
    string req_line = "GET /helloworld HTTP/1.1\r\n";
    read_stream << req_line;
    EXPECT_EQ(step(), 0);   // Req_line
    req->go();
    EXPECT_EQ(line_begin(), 0);     // Req_line
    EXPECT_EQ(line_end(), req_line.size());
    EXPECT_EQ(step(), 1);   // Headers

    reset();

    // with request line and headers
    iostream read_stream2(&read_buf());
    string headers = "k1:v1\r\nk2 : v2\r\nk3: v3\r\n";
    read_stream2 << req_line << headers;
    req->go();
    EXPECT_EQ(step(), 1);   // Headers
    EXPECT_EQ(line_begin(), req_line.size() + headers.size() - 8);
    EXPECT_EQ(line_end(), req_line.size() + headers.size());
    EXPECT_EQ(method(), 0); // GET
    EXPECT_EQ(path(), "/helloworld");
    EXPECT_EQ(header("k1"), "v1");
    EXPECT_EQ(header("k2"), "v2");
    EXPECT_EQ(header("k3"), "v3");

    reset();

    // with request line and headers and body
    iostream read_stream3(&read_buf());
    string body_msg = "1i12jie8fe8vyi32r01h9u12jh0??isef09aoiuh";
    string ex = "Content-Length: " + to_string(body_msg.size()) + "\r\n\r\n";
    read_stream3
        << req_line
        << headers
        << ex
        << body_msg;
    req->go();
    EXPECT_EQ(line_begin(), req_line.size() + headers.size() + ex.size());
    EXPECT_EQ(line_end(), req_line.size() + headers.size() + ex.size() + body_msg.size());
    EXPECT_EQ(step(), 4);   // OK
    EXPECT_EQ(body(), body_msg);

    reset();

    // with request line and headers and body
    iostream read_stream4(&read_buf());
    ex = "Content-Length: " + to_string(body_msg.size() + 2) + "\r\n\r\n";
    read_stream4 << req_line << headers << ex << body_msg;
    req->go();
    EXPECT_EQ(line_begin(), req_line.size() + headers.size() + ex.size()) << "should be: " << req_line[line_begin()];
    EXPECT_EQ(line_end(), req_line.size() + headers.size() + ex.size() + body_msg.size());
    EXPECT_EQ(step(), 2);   // Body
    EXPECT_EQ(body(), "");
    read_stream4 << "12";
    req->go();
    EXPECT_EQ(line_begin(), req_line.size() + headers.size() + ex.size()) << "should be: " << req_line[line_begin()];
    EXPECT_EQ(line_end(), req_line.size() + headers.size() + ex.size() + body_msg.size() + 2);
    EXPECT_EQ(step(), 4);   // Body
    EXPECT_EQ(body(), body_msg + "12");
}
