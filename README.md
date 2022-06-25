### TinyServer Async

基于 C++ 编写，运行于 Linux 环境下简单的 HTTP 服务器实现。

- 使用 **多线程线程池 + 异步IO** 实现用于提高性能。其中使用线程池处理计算型任务，如报文的解析；使用异步 IO 处理 IO 型任务，如网络通信。
- 使用 **有限状态机** 的设计模式驱动 HTTP 报文解析，将有限状态机作为单独的抽象类分离出来，相比于将解析逻辑写在一个类中的传统实现，这种实现逻辑更清晰，且具有更高的可读性和可拓展性。
- 实现了 **私有的缓冲结构 buffer**，隐藏了繁琐的网络和本地数据的解析，读取，拼接操作。实现了分批次处理数据，相较于一次性将文件读入内存，此实现占有更少的内存。


#### 依赖
- asio


#### 运行

安装所需工具：

`apt install gcc g++ make cmake libasio-dev`

进入到项目目录，编译：

`cmake .`

`make`

运行，其中参数含义 `./TinyServer_sync address port`，例：`./TinyServer_sync 127.0.0.1 2333`，将运行于 localhost 的 2333 端口。

注意，项目中使用了 `chroot(const char *path)` 函数，需要在 root 权限下运行。

#### 参考资料

- [Boost.Asio C++ 网络编程](https://mmoaay.gitbooks.io/boost-asio-cpp-network-programming-chinese/content/)
- C++ Concurrency in Action，中文名：C++并发编程实战