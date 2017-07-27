#include "Channel.h"
#include "EventLoop.h"

#include <iostream>

#include <assert.h>
#include <poll.h>

using namespace sjc;

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = POLLIN | POLLPRI;
const int Channel::kWriteEvent = POLLOUT;

Channel::Channel(EventLoop* loop, int fdArg)
  : loop_(loop),
    fd_(fdArg),
    events_(0),
    revents_(0),
    index_(-1),
    eventHandling_(false) {
}

Channel::~Channel() { assert(!eventHandling_); }

void
Channel::update() { loop_->updateChannel(this); }

void
Channel::handleEvent() {
  eventHandling_ = true;
  if (revents_ & POLLNVAL) { // 文件描述符没有打开
    std::clog << "Channel::handleEvent() POLLNVAL";
  }
  if ((revents_ & POLLHUP) && !(revents_ & POLLIN)) {
    std::clog << "Channel::handleEvent() POLLHUP";
    if (closeCallback_)
      closeCallback_();
  }
  if (revents_ & (POLLERR | POLLNVAL)) {
    if (errorCallback_)
      errorCallback_();
  }
  // POLLEDHUP
  // Stream socket peer closed connection or shutdown writing
  // half of connection
  if (revents_ & (POLLIN | POLLPRI | POLLRDHUP)) {
    std::clog << "Channel::handleEvent() POLLIN | POLLPRI | POLLRDHUP\n";
    if (readCallback_)
      readCallback_();
  }
  if (revents_ & POLLOUT) {
    if (writeCallback_)
      writeCallback_();
  }

  eventHandling_ = false;
}
