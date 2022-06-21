#include <iostream>
#include <asio.hpp>

using namespace std;
using namespace asio;
io_context ctx;

ip::tcp::socket so(ctx);
char buf[1024];
ssize_t size;

void f();

int main() {
    so.connect(ip::tcp::endpoint(ip::address::from_string("127.0.0.1"), 8888));
    f();
    ctx.run();
    cout.write(buf, size);
    so.close();
    return 0;
}

void f() {
    so.async_read_some(buffer(buf, 1024), [](const error_code &err, ssize_t sz) {
        cout << err.message() << endl;
        cout << sz << endl;
        size += sz;
        if (!err)
            f();
    });
}
