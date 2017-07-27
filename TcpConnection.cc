#include "TcpConnection.h"
#include "Channel.h"
#include "EventLoop.h"
#include "Socket.h"
#include "SocketsOps.h"

#include <iostream>

#include <errno.h>
#include <stdio.h>
#include <unistd.h> // write
#include <string.h> // strerr

using namespace sjc;


TcpConnection::TcpConnection(EventLoop* loop,
                             const std::string& name,
                             int connfd,
                             const InetAddress& localAddr,
                             const InetAddress& peerAddr)
                             : loop_(loop),
                               name_(name),
                               state_(kConnecting),
                               socket_(new Socket(connfd)),
                               channel_(new Channel(loop_, socket_->fd())),
                               localAddr_(localAddr),
                               peerAddr_(peerAddr) {
  channel_->setReadCallback(std::bind(&TcpConnection::handleRead, this));
  channel_->setWriteCallback(std::bind(&TcpConnection::handleWrite, this));
  channel_->setCloseCallback(std::bind(&TcpConnection::handleClose, this));
  channel_->setErrorCallback(std::bind(&TcpConnection::handleError, this));
  // 注意这里不能直接调用
  // channel_->enableReading();
  // 要确保连接已经建立
}

TcpConnection::~TcpConnection() {
}

// 向connfd发送消息 对外接口
void
TcpConnection::send(const std::string& message) {
  if (state_ == kConnected) {
    if (loop_->isInLoopThread()) {
      sendInLoop(message);
    } else {
      loop_->runInLoop(std::bind(&TcpConnection::sendInLoop, this, message));
    }
  }
}

// 向connfd发送消息 内部接口
// 首先尝试直接发送数据 如果一次发送完毕就不会Channel的enableWring事件
// 否则数据没有发送完 将剩余的数据放入outputBuffer_中 并enableWring事件
//
// 函数最先进行的判断是 如果当前outputBuffer_中有待发送的数据
// 那么就不可以尝试先发送了 否则会导致数据乱序
// 这时需要将数据直接交管给outputBuffer_发送
void
TcpConnection::sendInLoop(const std::string& message) {
  loop_->assertInLoopThread();
  ssize_t nwrote = 0;
  // connfd上没有关注过可写事件发生且输出缓冲区没有内容
  if (!channel_->isWriting() && outputBuffer_.readableBytes() == 0) {
    nwrote = ::write(channel_->fd(), message.data(), message.size());
    if (nwrote >= 0) {
      if (static_cast<size_t>(nwrote) < message.size()) { // 消息没有发送完
        std::clog << "TcpConnection::sendInLoop need to send more data\n";
      }
    } else {
      nwrote = 0;
      if (errno != EWOULDBLOCK) {
        std::cerr << "TcpConnection::sendInLoop\n";
      }
    }
  }

  assert(nwrote >= 0);
  if (static_cast<size_t>(nwrote) < message.size()) { // 处理消息没有发送完的情况
  // 每个IO事件上都只有一个线程负责 不会发生数据竞争
  outputBuffer_.append(message.data() + nwrote, message.size() - nwrote);
    if (!channel_->isWriting()) {
      channel_->enableWriting(); // 关注可写事件
    }
  }
}

void
TcpConnection::shutdown() {
  if (state_ == kConnected) {
    setState(kDisconnecting); // 这里只关闭写入端 但还可以接收数据
    loop_->runInLoop(std::bind(&TcpConnection::shutdownInLoop, this));
  }
}

void
TcpConnection::shutdownInLoop() {
  loop_->assertInLoopThread();
  if (!channel_->isWriting()) { // outputBuffer_中还有数据没有发送完时不能关闭
    socket_->shutdownWrite();
  }
}

void
TcpConnection::connectEstablished() {
  loop_->assertInLoopThread();
  assert(state_ == kConnecting);
  setState(kConnected);
  channel_->enableReading(); // 建立连接后 添加可读事件列表
  connectionCallback_(shared_from_this());
}


void
TcpConnection::connectDestoryed() {
  loop_->assertInLoopThread();
  assert(state_ == kConnected || state_ == kDisconnecting);
  setState(kDisconnected);
  channel_->disableAll();
  connectionCallback_(shared_from_this());

  loop_->removeChannel(channel_.get());
}

void
TcpConnection::handleRead() {
  int savedErrno = 0;
  ssize_t n = inputBuffer_.readFd(channel_->fd(), &savedErrno);
  if (n > 0) {
    messageCallback_(shared_from_this(), &inputBuffer_);
  } else if (n == 0) {
    std::clog << "TcpConnection::handleRead read 0\n";
    handleClose(); // 没有数据可读 是由EPOLLRDHUP触发的可读事件
  } else {
    errno = savedErrno;
    std::cerr << "TcpConnection::handleRead\n";
    handleError();
  }
}

void
TcpConnection::handleWrite() {
  std::clog << "handleWrite()\n";
  loop_->assertInLoopThread();
  if (channel_->isWriting()) {
    ssize_t n = ::write(channel_->fd(), outputBuffer_.peek(),
                        outputBuffer_.readableBytes());
    if (n > 0) {
      outputBuffer_.retrieve(n);
      if (outputBuffer_.readableBytes() == 0) {
        channel_->disableWriting();
        if (state_ == kDisconnecting) {
          shutdownInLoop();
        }
      } else {
        // std::clog << "TcpConnection::handleWrite need to write more data";
      }
    } else {
      std::cerr << "TcpConnection::handleWrite\n";
    }
  } else {
    std::cerr << "TcpConnection::handleWrite connection is down\n";
  }
}

void
TcpConnection::handleClose() {
  std::clog << "TcpConnection::handleClose\n";
  loop_->assertInLoopThread();
  assert(state_ == kConnected || state_ == kDisconnecting);
  channel_->disableAll();
  closeCallback_(shared_from_this());
}

void
TcpConnection::handleError() {
  int err = sockets::getSocketError(channel_->fd());
  char buf[32];
  std::clog << "TcpConnection::handleError [" << name_
            << "] - SO_ERROR = " << err << " " << strerror_r(err, buf, 32);
  // strerror_r为线程安全函数
}
