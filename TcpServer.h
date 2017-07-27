#ifndef SJC_TCPSERVER_H
#define SJC_TCPSERVER_H

#include "Callbacks.h"
#include "TcpConnection.h"
#include "Noncopyable.h"

#include <map>
#include <memory>
#include <string>

namespace sjc
{

class Acceptor;
class EventLoop;
class EventLoopThreadPool;

class TcpServer : noncopyable {
public:
  TcpServer(EventLoop* loop, const InetAddress& listenAddr);
  ~TcpServer();

  void start();

  void setThreadNum(int numThreads);

  void setConnectionCallback(const ConnectionCallback& cb) {
    connectionCallback_ = cb;
  }

  void setMessageCallback(const MessageCallback& cb) {
    messageCallback_ = cb;
  }

private:
  void newConnection(int connfd, const InetAddress& peerAddr);
  // 在acceptor_中 当监听套接字可读时 首先得到connfd peerAddr 然后回调该函数
  void removeConnection(const TcpConnectionPtr& conn);
  void removeConnectionInLoop(const TcpConnectionPtr& conn);

  typedef std::map<std::string, TcpConnectionPtr> ConnectionMap;

  EventLoop* loop_;
  const std::string name_;
  std::unique_ptr<Acceptor> acceptor_; //
  std::unique_ptr<EventLoopThreadPool> threadPool_;
  ConnectionCallback connectionCallback_; // 绑定到TcpConnection
  MessageCallback messageCallback_; // 绑定到TcpConnection
  bool started_;
  int nextCoundId_;
  ConnectionMap connections_;
};

}

#endif // SJC_TCPSERVER_H
