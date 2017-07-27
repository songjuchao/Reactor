#include "Buffer.h"
#include "SocketsOps.h"

#include <algorithm>

#include <errno.h>
#include <sys/uio.h>

using namespace sjc;

const char Buffer::kCRLF[] = "\r\n";

const size_t Buffer::kCheapPrepend;
const size_t Buffer::kInitialSize;

// readv(2)
// ssize_t readv(int fd, const struct iovec* iov, int iovcnt)
// The readv() system call reads iovcnt buffers from the file
// associated with the file descriptor fd into iov (scatter input)
// return the numbers of bytes readed
ssize_t Buffer::readFd(int fd, int* savedErrno) {
  char extrabuf[65536];
  struct iovec vec[2];
  const size_t writable = writableBytes();

  vec[0].iov_base = begin() + writerIndex_;
  vec[0].iov_len = writable;
  vec[1].iov_base = extrabuf;
  vec[1].iov_len = sizeof extrabuf;

  const int iovcnt = (writable < sizeof extrabuf) ? 2 : 1;
  const ssize_t n = sockets::readv(fd, vec, iovcnt);
  if (n < 0) {
    *savedErrno = errno;
  } else if (static_cast<size_t>(n) <= writable) {
    writerIndex_ += n;
  } else { // n > writable
    writerIndex_ = buffer_.size();
    append(extrabuf, n - writable); // 将读入extrabuf的内容追加到Buffer中
  }

  return n;
}

const char*
Buffer::findCRLF() const {
  const char* crlf = std::search(peek(), beginWrite(), kCRLF, kCRLF + 2);
  return crlf == beginWrite() ? NULL : crlf;
}
