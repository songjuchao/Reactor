#include "InetAddress.h"
#include "EventLoop.h"
#include "Acceptor.h"
#include "SocketsOps.h"

#include <assert.h>

using namespace sjc;

Acceptor::Acceptor(EventLoop* loop, const InetAddress& listenAddr)
  : loop_(loop),
    acceptSocket_(sockets::createNonblockingOrDie()),
    acceptChannel_(loop_, acceptSocket_.fd()),
    listenning_(false) {
  acceptSocket_.setReuseAddr(true);
  acceptSocket_.bindAddress(listenAddr);
  acceptChannel_.setReadCallback(std::bind(&Acceptor::handleRead, this));
}

void
Acceptor::listen() {
  loop_->assertInLoopThread();
  assert(!listenning_);
  listenning_ = true;
  acceptSocket_.listen();
  acceptChannel_.enableReading();
}

void
Acceptor::handleRead() {
  loop_->assertInLoopThread();
  InetAddress peerAddr(0);
  int connfd = acceptSocket_.accept(&peerAddr);
  if (connfd >= 0) {
    if (newConnectionCallback_) {
      newConnectionCallback_(connfd, peerAddr);
    } else {
      sockets::close(connfd);
    }
  }
}
