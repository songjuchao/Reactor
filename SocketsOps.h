#ifndef SJC_SOCKETSOPS_H
#define SJC_SOCKETSOPS_H

#include <arpa/inet.h>

namespace sjc
{
namespace sockets
{

ssize_t readv(int sockfd, const struct iovec* iov, int iovcnt);

// Creates a non-blocking socket file descriptor
// abort if any error
int createNonblockingOrDie();

void bindOrDie(int sockfd, const struct sockaddr_in& addr);
void listenOrDie(int sockfd);
int accept(int sockfd, struct sockaddr_in* addr);
void close(int sockfd);
void shutdownWrite(int sockfd);

void toHostPort(char* buf, size_t size, const struct sockaddr_in& addr);
void fromHostPort(const char* ip, uint16_t port, struct sockaddr_in* addr);

struct sockaddr_in getLocalAddr(int sockfd);

int getSocketError(int sockfd);

} // sockets
} // sjc

#endif // SJC_SOCKETSOPS_H
