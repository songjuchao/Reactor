#ifndef SJC_TCPCONNECTION_H
#define SJC_TCPCONNECTION_H

#include "Buffer.h"
#include "Callbacks.h"
#include "InetAddress.h"
#include "Noncopyable.h"

#include <memory>
#include <string>
#include <atomic>

namespace sjc
{

class Channel;
class EventLoop;
class Socket;

class TcpConnection : noncopyable,
                      public std::enable_shared_from_this<TcpConnection> {
public:
  // Constructs a TcpConnection with a connected sockfd
  TcpConnection(EventLoop* loop,
                const std::string& name,
                int connfd,
                const InetAddress& localAddr,
                const InetAddress& peerAddr);
  ~TcpConnection();

  EventLoop* getLoop() const { return loop_; }
  const std::string& name() const { return name_; }
  const InetAddress& localAddr() const { return localAddr_; }
  const InetAddress& peerAddr() const { return peerAddr_; }
  bool connected() const { return state_ == kConnected; }

  void send(const std::string& message);
  // 线程安全 可以跨线程调用
  void shutdown();

  void setConnectionCallback(const ConnectionCallback& cb) {
    connectionCallback_ = cb;
  }
  void setMessageCallback(const MessageCallback& cb) {
    messageCallback_ = cb;
  }
  void setCloseCallback(const CloseCallback& cb) {
    closeCallback_ = cb;
  }

  void connectEstablished(); // 建立连接
  void connectDestoryed(); // 连接断开

private:
  enum StateE { kConnecting, kConnected, kDisconnecting, kDisconnected, };

  void setState(StateE s) { state_ = s; }
  void handleRead();
  void handleWrite();
  void handleClose();
  void handleError();
  void sendInLoop(const std::string& message);
  void shutdownInLoop();

  EventLoop* loop_;
  std::string name_;
  std::atomic<StateE> state_;
  std::unique_ptr<Socket> socket_;
  std::unique_ptr<Channel> channel_;
  InetAddress localAddr_;
  InetAddress peerAddr_;
  ConnectionCallback connectionCallback_;
  MessageCallback messageCallback_;
  CloseCallback closeCallback_;
  Buffer inputBuffer_; // 输入缓冲区是用来接收从连接送达的消息
  Buffer outputBuffer_; // 输出缓冲区是用来向连接发送消息
};

}
#endif // SJC_TCPCONNECTION_H
