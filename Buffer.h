#ifndef SJC_NET_BUFFER_H
#define SJC_NET_BUFFER_H

#include "Copyable.h"

#include <vector>
#include <algorithm>
#include <string>

#include <assert.h>
#include <string.h>

namespace sjc
{

//
// +-------------------+----------------+----------------+
// | prependable bytes | readable bytes | writable bytes |
// |                   |    (CONTENT)   |                |
// +-------------------+----------------+----------------+
// |                   |                |                |
// 0       <=     readerIndex   <=  writerIndex   <=    size
//
class Buffer : public copyable {
public:
  static const size_t kCheapPrepend = 8;
  static const size_t kInitialSize = 1024;

  explicit Buffer(size_t initialSize = kInitialSize)
    : buffer_(kCheapPrepend + initialSize),
      readerIndex_(kCheapPrepend),
      writerIndex_(kCheapPrepend) {
    assert(readableBytes() == 0);
    assert(writableBytes() == initialSize);
    assert(prependableBytes() == kCheapPrepend);
  }

  // default copy-ctor move-ctor dtor and assignment are okay

  void swap(Buffer& rhs) {
    buffer_.swap(rhs.buffer_);
    std::swap(readerIndex_, rhs.readerIndex_);
    std::swap(writerIndex_, rhs.writerIndex_);
  }

  size_t readableBytes() const { return writerIndex_ - readerIndex_; }
  size_t writableBytes() const { return buffer_.size() - writerIndex_; }
  size_t prependableBytes() const { return readerIndex_; }

  // 返回可读的位置
  const char* peek() const { return begin() + readerIndex_; }
  const char* findCRLF() const;

  // Read data directly into buffer_
  // implement with readv(2)
  // return result of read(2) errno is saved
  ssize_t readFd(int fd, int* savedErrno);

  // 读取len字节后 保持下标
  void retrieve(size_t len) {
    assert(len <= readableBytes());
    if (len < readableBytes()) {
      readerIndex_ += len;
    } else { // len == readableBytes()
      retrieveAll();
    }
  }

  // 将数据全部读取后 恢复下标为初始状态
  void retrieveAll() {
    readerIndex_ = kCheapPrepend;
    writerIndex_ = readerIndex_;
  }

  // 将中内容以string返回 重置缓冲区
  std::string retrieveAsString() {
    std::string str(peek(), readableBytes());
    retrieveAll();
    return str;
  }

  void append(const char* data, size_t len) {
    ensureWritableBytes(len);
    std::copy(data, data + len, beginWrite());
    hasWritten(len);
  }

  void ensureWritableBytes(size_t len) {
    if (writableBytes() < len)
      makeSpace(len);

    assert(writableBytes() >= len);
  }

  char* beginWrite() { return begin() + writerIndex_; }

  const char* beginWrite() const { return begin() + writerIndex_; }

  void hasWritten(size_t len) {
    assert(len <= writableBytes());
    writerIndex_ += len;
  }

private:
  // date() c_str()仅仅为const char*
  char* begin() { return &*buffer_.begin(); } // 对iterator做一次转换
  const char* begin() const { return &*buffer_.cbegin(); }

  void makeSpace(size_t len) {
    if (writableBytes() + prependableBytes() < len + kCheapPrepend) {
      buffer_.resize(writerIndex_ + len); // 保证可写位置后至少有len
    } else { // 在缓冲区尾部挪出足够的空间
      size_t readable = readableBytes();
      std::copy(begin() + readerIndex_, begin() + writerIndex_,
                begin() + kCheapPrepend);
      readerIndex_ = kCheapPrepend;
      writerIndex_ = readerIndex_ + readable;
      assert(readable == readableBytes());
    }
  }

private:
  std::vector<char> buffer_;
  size_t readerIndex_; // 读开始的下标 避免容量变化之后失效
  size_t writerIndex_; // 写开始的下标

  static const char kCRLF[]; // "\r\n\r\n"
};

} // sjc

#endif // SJC_BUFFER_H
