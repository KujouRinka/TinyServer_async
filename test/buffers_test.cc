#include <gtest/gtest.h>

#include <unistd.h>

#include "common/buffer.h"
#include "connection/connection.h"

class ResponseBufferTest : public ::testing::Test {
 protected:
  void SetUp() override {
    Response response;
    response.version = "HTTP/1.1";
    response.status = "200";
    response.reason = "OK";
    response.headers = {
      {"Content-Type", "text/plain"},
      {"Content-Length", "0"},
    };
    buffer = new ResponseBlockBuffer(response);
  }

  void TearDown() override {
    delete buffer;
  }

  void reset() {
    TearDown();
    SetUp();
  }

  string &raw_buffer() {
    return buffer->data;
  }

  auto idx() {
    return buffer->idx;
  }

  ResponseBlockBuffer *buffer;
};

TEST_F(ResponseBufferTest, responseBufferTest) {
  string res1 =
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/plain\r\n"
    "Content-Length: 0\r\n"
    "\r\n";
  string res2 =
    "HTTP/1.1 200 OK\r\n"
    "Content-Length: 0\r\n"
    "Content-Type: text/plain\r\n"
    "\r\n";
  ASSERT_TRUE(raw_buffer() == res1 || raw_buffer() == res2);
  ASSERT_EQ(idx(), 0);

  string ret1 = buffer->get(2);
  ASSERT_EQ(ret1, "HT");
  ASSERT_EQ(idx(), 2);
  string ret2 = buffer->get(15);
  ASSERT_EQ(ret2, "TP/1.1 200 OK\r\n");
  ASSERT_EQ(idx(), 17);
  string ret3 = buffer->get(1024);
  ASSERT_EQ(idx(), res1.size());
}

class FileIterBufferTest : public ::testing::Test {
 protected:
  void SetUp() override {
    ifstream ifs("file.txt");
    ASSERT_TRUE(ifs.is_open());
    buffer = new FileIterBlockBuffer(std::move(ifs));
  }

  void TearDown() override {
    delete buffer;
  }

  void reset() {
    TearDown();
    SetUp();
  }

  FileIterBlockBuffer *buffer;
};

TEST_F(FileIterBufferTest, fileIterBufferTest) {
  string s;
  s = buffer->get(8);
  ASSERT_EQ(s.size(), 8);
  ASSERT_EQ(s, "Ravenous");
  s = buffer->get(8);
  ASSERT_EQ(s.size(), 8);
  ASSERT_EQ(s, ", and no");
  s = buffer->get(8);
  ASSERT_EQ(s.size(), 7);
  ASSERT_EQ(s, "w faint");
}

class FileDescriptorBufferTest : public ::testing::Test {
 protected:
  void SetUp() override {
    fd = open("file.txt", O_RDONLY);
    ASSERT_NE(fd, -1);
    buffer = new FileDescriptorBlockBuffer(fd);
  }

  void TearDown() override {
    delete buffer;
    close(fd);
  }

  void reset() {
    TearDown();
    SetUp();
  }

  FileDescriptorBlockBuffer *buffer;
  int fd;
};

TEST_F(FileDescriptorBufferTest, fileDescriptorBufferTest) {
  string s;
  s = buffer->get(8);
  ASSERT_EQ(s.size(), 8);
  ASSERT_EQ(s, "Ravenous");
  s = buffer->get(8);
  ASSERT_EQ(s.size(), 8);
  ASSERT_EQ(s, ", and no");
  s = buffer->get(8);
  ASSERT_EQ(s.size(), 7);
  ASSERT_EQ(s, "w faint");
}

class ComposeBufferTest : public ::testing::Test {
 protected:
  void SetUp() override {
    Response response;
    response.version = "HTTP/1.1";
    response.status = "200";
    response.reason = "OK";
    response.headers = {
      {"Content-Type", "text/plain"},
      {"Content-Length", "46"},
    };
    auto resp_buf = new ResponseBlockBuffer(response);
    auto file_buf = new FileIterBlockBuffer(ifstream("file.txt"));
    auto fd_buf = new FileDescriptorBlockBuffer(open("file.txt", O_RDONLY));
    buffer = new ComposeBuffer({resp_buf, file_buf, fd_buf});
  }

  void TearDown() override {
    delete buffer;
  }

  void reset() {
    TearDown();
    SetUp();
  }

  ComposeBuffer *buffer;
};

TEST_F(ComposeBufferTest, composeBufferTest) {
  string s;
  string tmp;
  while (!(tmp = buffer->get(17)).empty()) {
    s += tmp;
  }
  string res1 =
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/plain\r\n"
    "Content-Length: 46\r\n"
    "\r\n"
    "Ravenous, and now faint"
    "Ravenous, and now faint";
  string res2 =
    "HTTP/1.1 200 OK\r\n"
    "Content-Length: 46\r\n"
    "Content-Type: text/plain\r\n"
    "\r\n"
    "Ravenous, and now faint"
    "Ravenous, and now faint";
  EXPECT_TRUE(s == res1 || s == res2) << "s: " << s;
}
