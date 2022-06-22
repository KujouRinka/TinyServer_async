#ifndef TINYSERVER_ASYNC_STATE_H
#define TINYSERVER_ASYNC_STATE_H

#include <string>
#include <regex>

class Connection;
class ParseReqTest;

class State {
public:
    explicit State(Connection *conn);
    virtual void go() = 0;

protected:
    Connection *_conn;
};

class ParseReq : public State {
    friend class ParseReqTest;
public:
    explicit ParseReq(Connection *conn);
    void go() override;

private:
    enum STEP {
        Req_line = 0,
        Headers,
        Body,
        Bad,
        OK,
    };
    STEP step;
    std::string req_buffer;

    static std::regex req_re;
    static std::regex headers_re;

    int line_begin;
    int line_end;
    int content_length;
    int next_begin;

private:
    // update line_begin and line_end for next line.
    // return true if success.
    bool nextLine();
    void req();
    void headers();
    void body();
};

class ParseRead : public State {
public:
    explicit ParseRead(Connection *conn);
    void go() override;
};

class ParseWrite : public State {
public:
    explicit ParseWrite(Connection *conn);
    void go() override;
};

class Bad : public State {
public:
    explicit Bad(Connection *conn);
    void go() override;
};

#endif //TINYSERVER_ASYNC_STATE_H