#include <gtest/gtest.h>
#include <string>

#include "connection/state.h"

using namespace std;

class ParseReqTest : public ::testing::Test {
protected:
    void SetUp() override {
        req = new ParseReq(nullptr);

    }

    void TearDown() override {
        delete req;
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

    void reset() {
        delete req;
        req = new ParseReq(nullptr);
    }

    ParseReq *req;
};

TEST_F(ParseReqTest, parseReqTest) {
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
