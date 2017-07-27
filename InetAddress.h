#ifndef SJC_INETADDRESS_H
#define SJC_INETADDRESS_H

#include "Copyable.h"

#include <string>

#include <netinet/in.h>

namespace sjc
{

// Wrapper of sockaddr_in
class InetAddress : public copyable { // 值语义
public:
  explicit InetAddress(uint16_t port);
  // ip should be "1.2.3.4"
  InetAddress(const std::string& ip, uint16_t port);

  // Mostly used when accepting new connections
  InetAddress(const struct sockaddr_in addr)
    : addr_(addr) {
  }

  std::string toHostPort() const;
  // default copy/assignment are Okay

  const struct sockaddr_in& getSockAddrInet() const { return addr_; }
  void setSockAddrInet(const struct sockaddr_in addr) { addr_ = addr; }

private:
  struct sockaddr_in addr_;
};


}

#endif // SJC_INETADDRESS_H
