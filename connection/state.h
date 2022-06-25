#ifndef TINYSERVER_ASYNC_STATE_H
#define TINYSERVER_ASYNC_STATE_H

#include <string>
#include <memory>
#include <regex>

using namespace std;

class Connection;
class ParseLineTest;
class ParseReqTest;

class State {
 public:
  explicit State(Connection *conn);
  virtual void go(shared_ptr<Connection> holder) = 0;

 protected:
  Connection *_conn;
};

class ParseReq : public State {
  friend class ParseLineTest;
  friend class ParseReqTest;
 public:
  explicit ParseReq(Connection *conn);
  void go(shared_ptr<Connection> holder) override;

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
  int next_begin;
  int content_length;
  bool have_content_length;   // TODO: temporary solution

 private:
  // update line_begin and line_end for next line.
  // return true if success.
  bool nextLine();
  void req();
  void headers();
  void body(shared_ptr<Connection> holder);
};

class Responding : public State {
 public:
  explicit Responding(Connection *conn);
  void go(shared_ptr<Connection> holder) override;

 private:
  std::ostream os;
};

class Bad : public State {
 public:
  explicit Bad(Connection *conn);
  void go(shared_ptr<Connection> holder) override;
};

#endif //TINYSERVER_ASYNC_STATE_H
