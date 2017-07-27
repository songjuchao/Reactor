#ifndef SJC_SOCKET_H
#define SJC_SOCKET_H

#include "Noncopyable.h"

namespace sjc
{

class InetAddress;

// Wrapper of socket file descriptor
// It close the sockfd when destructs RAII
class Socket : public noncopyable {
public:
  explicit Socket(int sockfd)
  : sockfd_(sockfd) {
  }

  ~Socket();

  int fd() const { return sockfd_; }
  //
  void bindAddress(const InetAddress& localaddr);
  void listen();

  // On error, return -1, and *peeraddr is untouched
  int accept(InetAddress* peeraddr);

  void setReuseAddr(bool on);
  void shutdownWrite();

private:
  const int sockfd_;
};

}

#endif // SJC_SOCKET_H
