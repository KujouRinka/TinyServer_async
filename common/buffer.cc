#include "buffer.h"

#include <algorithm>

#include "connection/connection.h"

BlockBuffer::~BlockBuffer() = default;

ComposeBuffer::ComposeBuffer() = default;

ComposeBuffer::ComposeBuffer(initializer_list<BlockBuffer *> blocks)
    : _buffers(blocks) {}

string ComposeBuffer::get(size_t size) {
    string ret;
    while (!_buffers.empty() && (ret = _buffers.front()->get(size)).empty()) {
        delete _buffers.front();
        _buffers.pop();
        if (_buffers.empty())
            break;
    }
    return ret;
}

void ComposeBuffer::appendBuffer(BlockBuffer *buffer) {
    _buffers.push(buffer);
}

ResponseBlockBuffer::ResponseBlockBuffer(const Response &response)
    : idx(0) {
    // I guess the usual size of response is about 128 bytes
    data.reserve(128);
    data.append(response.version).append(" ")
        .append(response.status).append(" ")
        .append(response.reason).append("\r\n");
    for_each(response.headers.begin(), response.headers.end(),
             [&](auto &header) {
                 data.append(header.first).append(": ")
                     .append(header.second).append("\r\n");
             });
    data.append("\r\n");
}

string ResponseBlockBuffer::get(size_t size) {
    string s = data.substr(idx, size);
    idx += s.size();
    return std::move(s);
}

FileIterBlockBuffer::FileIterBlockBuffer(ifstream ifs)
    : fs(std::move(ifs)) {}

// TODO:
string FileIterBlockBuffer::get(size_t size) {
    string ret;
    ret.resize(size);
    fs.read(ret.data(), size);
    ret.resize(fs.gcount());
    return ret;
}

FileDescriptorBlockBuffer::FileDescriptorBlockBuffer(int fd)
    : _fd(fd) {}

FileDescriptorBlockBuffer::~FileDescriptorBlockBuffer() {
    close(_fd);
}

string FileDescriptorBlockBuffer::get(size_t size) {
    string ret;
    ret.resize(size);
    auto read_size = read(_fd, ret.data(), size);
    if (read_size == 0)
        return "";
    // TODO: what to do for error?
    // else if (read_size == -1)
    //     throw runtime_error("read error");
    ret.resize(read_size);
    return std::move(ret);
}
