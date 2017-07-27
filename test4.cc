#include "EventLoop.h"
#include "Acceptor.h"
#include "InetAddress.h"
#include "SocketsOps.h"

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h> // getpid
#include <string.h>

void
newConnection(int connfd, const sjc::InetAddress& peerAddr) {
  printf("newConnection() : accepted a new connection from %s\n",
         peerAddr.toHostPort().c_str());
  ::write(connfd, "Hello, world!\n", strlen("Hello, world!\n"));
  sjc::sockets::close(connfd);
}

int
main() {
  printf("main() : pid = %d\n", getpid());

  sjc::EventLoop loop;
  sjc::InetAddress listenAddr(9981);
  sjc::Acceptor acceptor(&loop, listenAddr);
  acceptor.setNewConnectionCallback(newConnection);
  acceptor.listen();

  loop.loop();
}
