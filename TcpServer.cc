#include "TcpServer.h"
#include "Acceptor.h"
#include "EventLoop.h"
#include "SocketsOps.h"
#include "EventLoopThreadPool.h"

#include <iostream>

#include <stdio.h>

using namespace sjc;

TcpServer::TcpServer(EventLoop* loop, const InetAddress& listenAddr)
  : loop_(loop),
    name_(listenAddr.toHostPort()),
    acceptor_(new Acceptor(loop, listenAddr)),
    threadPool_(new EventLoopThreadPool(loop)),
    started_(false),
    nextCoundId_(1) {
  // 被绑定的函数需要提供2个参数 占位符
  acceptor_->setNewConnectionCallback(
    std::bind(&TcpServer::newConnection, this, std::placeholders::_1,
                                               std::placeholders::_2));
}

TcpServer::~TcpServer() { }

void
TcpServer::setThreadNum(int numThreads) {
  assert(numThreads >= 0);
  threadPool_->setThreadNum(numThreads);
}

void
TcpServer::start() {
  if (!started_) {
    started_ = true;
  }

  if (!acceptor_->listenning()) {
    loop_->runInLoop(std::bind(&Acceptor::listen, acceptor_.get()));
  }
}

void
TcpServer::newConnection(int connfd, const InetAddress& peerAddr) {
  loop_->assertInLoopThread();
  char buf[32];
  snprintf(buf, sizeof buf, "#%d", nextCoundId_);
  ++nextCoundId_;
  std::string connName = name_ + buf;

  std::clog << "TcpServer::newConnetion [" << name_
            << "] - new connection [" << connName
            << "] from " << peerAddr.toHostPort() << "\n";
  InetAddress localAddr(sockets::getLocalAddr(connfd));

  EventLoop* ioLoop = threadPool_->getNextLoop();
  TcpConnectionPtr conn = std::make_shared<TcpConnection>
                          (loop_, connName, connfd, localAddr, peerAddr);
  connections_[connName] = conn;
  conn->setConnectionCallback(connectionCallback_);
  conn->setMessageCallback(messageCallback_);
  conn->setCloseCallback(std::bind(&TcpServer::removeConnection, this,
                                   std::placeholders::_1));
  // 关闭连接的过程非常复杂 但是每一个函数回调都必须
  // 首先执行TcpServer的回调 将TcpConnectionPtr连接记录erase
  // 接着必须在每个线程执行完io事件回调之后才能做最后的清除工作
  ioLoop->runInLoop(std::bind(&TcpConnection::connectEstablished, conn));
  // 在线程的EventLoop中执行回调
}

void
TcpServer::removeConnection(const TcpConnectionPtr& conn) {
  loop_->runInLoop(std::bind(&TcpServer::removeConnectionInLoop, this, conn));
}

void
TcpServer::removeConnectionInLoop(const TcpConnectionPtr& conn) {
  loop_->assertInLoopThread();
  std::clog << "TcpServer:removeConnection [" << name_
            << "] - connection " << conn->name() << "\n";
  size_t n = connections_.erase(conn->name());
  assert(n == 1);
  EventLoop* ioLoop = conn->getLoop();
  // 这里移除连接时必须要等到该连接上的所有事件都响应结束
  // 否则移除连接时connfd上的Channel还在handleEvent事件中 但该对象却被销毁了
  // 即要等待Channel上的handleEvent都执行完
  // 所以将最后的移除函数放置在doPendingFunctors队列中 最后等待执行
  ioLoop->queueInLoop(std::bind(&TcpConnection::connectDestoryed, conn));
  // ioLoop->runInLoop(std::bind(&TcpConnection::connectDestoryed, conn));
}
