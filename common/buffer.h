#ifndef TINYSERVER_ASYNC_BUFFER_H
#define TINYSERVER_ASYNC_BUFFER_H

#include <fstream>
#include <queue>
#include <utility>

using namespace std;

class Response;

using buffer_range = pair<char *, char *>;

// ---------------------------------------------
// Buffer interface
class BlockBuffer {
public:
    virtual ~BlockBuffer();
    virtual string get(size_t size) = 0;
};

// ---------------------------------------------
// CompressedBuffer
class ComposeBuffer : public BlockBuffer {
public:
    ComposeBuffer();
    ComposeBuffer(initializer_list<BlockBuffer *> blocks);
    string get(size_t size) override;
    void appendBuffer(BlockBuffer *buffer);

private:
    // order in _buffers matters
    queue<BlockBuffer *> _buffers;
};

// ---------------------------------------------
// ResponseBlockBuffer
class ResponseBufferTest;
class ResponseBlockBuffer : public BlockBuffer {
    friend class ResponseBufferTest;
public:
    explicit ResponseBlockBuffer(const Response &response);
    string get(size_t size) override;

private:
    string::size_type idx;
    string data;
};

// ---------------------------------------------
// FileBlockBuffer
// TODO:
class FileIterBlockBuffer : public BlockBuffer {
public:
    explicit FileIterBlockBuffer(ifstream ifs);
    string get(size_t size) override;

private:
    ifstream fs;
};

// ---------------------------------------------
// FileDescriptorBlockBuffer
class FileDescriptorBlockBuffer : public BlockBuffer {
public:
    explicit FileDescriptorBlockBuffer(int fd);
    ~FileDescriptorBlockBuffer();
    string get(size_t size) override;

private:
    int _fd;
};

#endif //TINYSERVER_ASYNC_BUFFER_H
