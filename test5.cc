#include "EventLoop.h"
#include "TcpServer.h"
#include "InetAddress.h"

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

void
onConnection(const sjc::TcpConnectionPtr& conn) {
  if (conn->connected()) { // 建立连接时
    printf("onConnection() : new connection [%s] from %s\n",
           conn->name().data(),
           conn->peerAddr().toHostPort().data());
  } else { // 连接断开时
    printf("onConnection() : connetion [%s] is down\n",
           conn->name().data());
  }
}

void
onMessage(const sjc::TcpConnectionPtr& conn, sjc::Buffer* buf) {
  // Z a following integer conversion corresponds to a size_t or ssize_t
  printf("onMessage() : received %Zd bytes from connection [%s]\n",
         buf->readableBytes(),
         conn->name().data());

  printf("onMessage() : [%s]\n", buf->retrieveAsString().data());
}

int
main() {
  printf("main() : pid = %d\n", getpid());

  sjc::EventLoop loop;
  sjc::InetAddress listenAddr(9981);

  sjc::TcpServer server(&loop, listenAddr);
  server.setConnectionCallback(onConnection);
  server.setMessageCallback(onMessage);
  server.start();

  loop.loop();
}
