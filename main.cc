#include <iostream>
#include <memory>
#include <cstring>
#include <asio.hpp>

#include "connection/connection.h"
#include "threadpool/threadpool.h"

using namespace std;
using namespace asio;

io_context ctx;
ip::tcp::endpoint listen_address;
ip::tcp::acceptor ac(ctx);
ip::tcp::socket so(ctx);
ThreadPool pool;

void async_listen() {
  so = ip::tcp::socket(ctx);
  ac.async_accept(so, [](const error_code &err) {
    if (!err) {
      make_shared<Connection>(std::move(so))->inRead();
    }
    async_listen();
  });
}

int main(int argc, char **argv) {
  if (argc != 3) {
    cerr << "invalid arguments: Usage: ./TinyServer address port" << endl;
    exit(1);
  }
  // root privilege is required to call chroot(...)
  if (int err = chroot("root"); err == -1) {
    cerr << strerror(errno) << ": " << "calling chroot(...) needs root privilege" << endl;
    exit(1);
  }
  uint16_t port;
  try {
    port = stoi(argv[2], nullptr);
  } catch (const exception &e) {
    cerr << "invalid port" << endl;
    exit(1);
  }
  cout << "running at: " << argv[1] << ":" << argv[2] << endl;
  listen_address = ip::tcp::endpoint(ip::address::from_string(argv[1]), port);
  ac = ip::tcp::acceptor(ctx, listen_address);
  async_listen();
  ctx.run();
  return 0;
}
