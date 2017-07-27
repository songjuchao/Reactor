#include "SocketsOps.h"

#include <stdexcept>

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <strings.h> // bzero
#include <sys/socket.h>
#include <unistd.h>

using namespace sjc;

template<typename To, typename From>
inline To implicit_cast(From const& f) {
  return f;
}

typedef struct sockaddr SA;

const SA* sockaddr_cast(const struct sockaddr_in* addr) {
  return static_cast<const SA*>(implicit_cast<const void*>(addr));
}

SA* sockaddr_cast(struct sockaddr_in* addr) {
  return static_cast<SA*>(implicit_cast<void*>(addr));
}

ssize_t
sockets::readv(int sockfd, const struct iovec* iov, int iovcnt) {
  return ::readv(sockfd, iov, iovcnt);
}

int
sockets::createNonblockingOrDie() {
  int sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC,
                        IPPROTO_TCP);
  if (sockfd < 0) {
    throw std::runtime_error("sockets::createNonblockingOrDie");
  }

  return sockfd;
}

void
sockets::bindOrDie(int sockfd, const struct sockaddr_in& addr) {
int ret = ::bind(sockfd, sockaddr_cast(&addr), sizeof addr);
if (ret < 0)
  throw std::runtime_error("sockets::bindOrDie");
}

void
sockets::listenOrDie(int sockfd) {
  int ret = ::listen(sockfd, SOMAXCONN);
  if (ret < 0)
    throw std::runtime_error("sockets::listenOrDie");
}

int
sockets::accept(int sockfd, struct sockaddr_in* addr) {
  socklen_t addrlen = sizeof *addr;
  int connfd = ::accept4(sockfd, sockaddr_cast(addr), &addrlen,
                         SOCK_NONBLOCK | SOCK_CLOEXEC);

  if (connfd < 0) {
    throw std::runtime_error("sockets::accept");
  }

  return connfd;
}

void
sockets::close(int sockfd) {
  if (::close(sockfd) < 0) {
    throw std::runtime_error("sockets::close");
  }
}

void
sockets::shutdownWrite(int sockfd) { // 关闭连接写入端
  if (::shutdown(sockfd, SHUT_WR) < 0) {
    throw std::runtime_error("sockets::shutdownWrite");
  }
}

void
sockets::toHostPort(char* buf, size_t size, const struct sockaddr_in& addr) {
  char host[INET_ADDRSTRLEN] = "INVALID"; // The maximum lengths
  ::inet_ntop(AF_INET, &addr.sin_addr, host, sizeof host);
  uint16_t port = ::ntohs(addr.sin_port);
  snprintf(buf, size, "%s:%u", host, port);
}

void
sockets::fromHostPort(const char* ip, uint16_t port, struct sockaddr_in* addr) {
  addr->sin_family = AF_INET;
  addr->sin_port = ::htons(port);
  if (::inet_pton(AF_INET, ip, &addr->sin_addr) <= 0) {
    throw std::runtime_error("sockets::fromHostPort");
  }
}

struct sockaddr_in
sockets::getLocalAddr(int sockfd) {
  struct sockaddr_in localaddr;
  bzero(&localaddr, sizeof localaddr);
  socklen_t addrlen = sizeof localaddr;
  if (::getsockname(sockfd, sockaddr_cast(&localaddr), &addrlen) < 0) {
    throw std::runtime_error("sockets::getLocalAddr");
  }
}

int
sockets::getSocketError(int sockfd) {
  int optval;
  socklen_t optlen = sizeof optval;
  if (::getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0) {
    return errno;
  } else {
    return optval;
  }
}
