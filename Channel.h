#ifndef SJC_CHANNEL_H
#define SJC_CHANNEL_H

#include "Noncopyable.h"

#include <functional>
#include <iostream>

namespace sjc
{

class EventLoop;

class Channel : noncopyable {
public:
  typedef std::function<void()> EventCallback;
  typedef std::function<void()> ReadEventCallback;

  Channel(EventLoop* loop, int fd);
  ~Channel();

  void handleEvent();
  void setReadCallback(const ReadEventCallback& cb) { readCallback_ = cb; }
  void setWriteCallback(const EventCallback& cb) { writeCallback_ = cb; }
  void setErrorCallback(const EventCallback& cb) { errorCallback_ = cb; }
  void setCloseCallback(const EventCallback& cb) { closeCallback_ = cb; }

  int fd() const { return fd_; }
  int events() const { return events_; }
  void set_revents(int revt) { revents_ = revt; }
  bool isNoneEvent() const { return events_ == kNoneEvent; }

  void enableReading() { events_ |= kReadEvent; update(); }
  void enableWriting() { events_ |= kWriteEvent; std::clog << "Channel::enableWriting\n"; update(); }
  void disableWriting() { events_ &= ~kWriteEvent; update(); }
  void disableAll() { events_ = kNoneEvent; update(); }
  bool isWriting() const { return events_ & kWriteEvent; } // 可写

  // For Poller
  int index() { return index_; }
  void set_index(int idx) { index_ = idx; }

  EventLoop* ownerLoop() { return loop_; }

private:
  void update();

private:
  static const int kNoneEvent;
  static const int kReadEvent;
  static const int kWriteEvent;

  EventLoop* loop_;
  const int fd_; // 负责的文件描述符
  int events_; // 监听的事件
  int revents_; // 发生的事件
  int index_; // Used by Poller PollFdList下标

  bool eventHandling_;

  ReadEventCallback readCallback_; // 可读事件回调函数
  EventCallback writeCallback_; // 可写事件
  EventCallback errorCallback_; // 错误事件
  EventCallback closeCallback_; // 关闭事件
};

}

#endif // SJC_CHANNEL_H
