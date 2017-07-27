#ifndef SJC_ACCEPTOR_H
#define SJC_ACCEPTOR_H

#include "Channel.h"
#include "Socket.h"

#include <functional>

namespace sjc
{

class InetAddress;
class EventLoop;

class Acceptor : public noncopyable {
public:
  typedef std::function<void(int connfd, const InetAddress&)>
          NewConnectionCallback; // 注意该回调函数的参数设置
                                 // 新建连接时会有返回连接套接字和连接端的地址信息
  // listenAddr 提供绑定到监听套接字上的地址信息
  Acceptor(EventLoop* loop, const InetAddress& listenAddr);

  void setNewConnectionCallback(NewConnectionCallback cb) {
    newConnectionCallback_ = cb;
  }

  bool listenning() const { return listenning_; }
  void listen();

private:
  void handleRead();

private:
  Socket acceptSocket_; // 监听套接字
  EventLoop* loop_;
  Channel acceptChannel_;
  NewConnectionCallback newConnectionCallback_; // 建立新连接的回调函数
  bool listenning_;
};

}

#endif // SJC_ACCEPTOR_H
