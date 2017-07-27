#include <iostream>

#include "EventLoop.h"
#include "Channel.h"
#include "Poller.h"

#include <stdexcept> // runtime_error
#include <iostream> // clog

#include <assert.h>
#include <sys/eventfd.h>
#include <unistd.h>
#include <signal.h> // signal

using namespace sjc;

thread_local EventLoop* t_loopInThisThread = 0;
const int kPollTimeMs = 10000;

// int eventfd(unsigned int inival, int flags);
// creates an "eventfd object" that can be as an event wait/notify mechanism
// by user-space applications of events.
// eventfd是有内核维护的64b大小的数
// write调用增加这个值
// read调用读出这个值
// 写入时不能超过最大值 也不能读出0 其他细节可以man一下
static int createEventfd() {
  int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
  if (evtfd < 0) {
    std::cerr << "Failed in eventfd";
    abort();
  }

  return evtfd;
}

class IgnoreSigPipe {
public:
  IgnoreSigPipe() {
    ::signal(SIGPIPE, SIG_IGN);
  }
};

IgnoreSigPipe initObj; // 忽略SIGPIPE

EventLoop::EventLoop()
  : // looping_(false),
    quit_(false),
    callingPendingFunctors_(false),
    tid_(std::this_thread::get_id()),
    wakeupFd_(createEventfd()),
    poller_(new Poller(this)),
    wakeupChannel_(new Channel(this, wakeupFd_)) {
    if (t_loopInThisThread) {
      std::cerr << "Another EventLoop " << t_loopInThisThread
                << "exists in this thread " << tid_;
    } else {
      t_loopInThisThread = this;
    }
    wakeupChannel_->setReadCallback(std::bind(&EventLoop::handleRead, this));
    wakeupChannel_->enableReading();
}

EventLoop::~EventLoop() {
  // assert(!looping_);
  ::close(wakeupFd_);
  t_loopInThisThread = NULL;
}

void
EventLoop::loop() {
  // assert(!looping_);
  assertInLoopThread();
  // looping_ = true;
  quit_ = false;

  while (!quit_) {
    activeChannels_.clear();
    poller_->poll(kPollTimeMs, &activeChannels_);
    for (auto it = activeChannels_.begin(); it != activeChannels_.end(); ++it) {
      (*it)->handleEvent();
    }
     doPendingFunctors();
  }

  std::clog << "EventLoop " << this << " stop looping";
  // looping_ = false;
}

void
EventLoop::quit() {
  assert(!quit_);
  quit_ = true;
  if (!isInLoopThread()) {
    wakeup();
  }
}

void
EventLoop::runInLoop(const Functor& cb) {
  if (isInLoopThread()) {
    cb();
  } else {
    queueInLoop(cb);
  }
}

void
EventLoop::queueInLoop(const Functor& cb) {
  // throw std::runtime_error("EventLoop::queueInLoop() not implements");
  {
    std::lock_guard<std::mutex> lock(mutex_);
    pendingFuncors.push_back(cb);
  }

  if (!isInLoopThread() || callingPendingFunctors_) {
    // 访问callingPendingFunctors_变量时是否该加锁
    // doPendingFunctors函数中会改变该变量
    // 不需要加锁
    // 实际上当第一个判断为false 即处于调用EventLoop线程中时
    // 几乎可以肯定本函数是在事件回调中或者pendingFuncors中调用
    // 即不会出现对callingPendingFunctors_的数据竞争 其实发生竞争也没事...
    wakeup();
  }
}

void
EventLoop::updateChannel(Channel* channel) {
  assert(channel->ownerLoop() == this);
  assertInLoopThread();
  poller_->updateChannel(channel);
}

void
EventLoop::removeChannel(Channel* channel) {
  assert(channel->ownerLoop() == this);
  assertInLoopThread();
  poller_->removeChannel(channel);
}

void
EventLoop::abortNotInLoopThread() {
  throw std::runtime_error("abortNotInLoopThread");
}

void
EventLoop::handleRead() {
  uint64_t one = 1;
  size_t n = ::read(wakeupFd_, &one, sizeof one);
  if (n != sizeof one) {
    std::cerr << "EventLoop::handleRead reads " << n << " bytes instread of 8\n";
  }
}

void
EventLoop::wakeup() {
  uint64_t one = 1;
  ssize_t n = ::write(wakeupFd_, &one, sizeof one);
  if (n != sizeof one) {
    std::cerr << "EventLoop::wakeup writes " << n << " bytes instread of 8\n";
  }
}

void
EventLoop::doPendingFunctors() {
  callingPendingFunctors_ = true;
  std::vector<Functor> functors;

  // 减少临界区长度
  {
    std::lock_guard<std::mutex> lock(mutex_);
    functors.swap(pendingFuncors);
  }

  for (int i = 0; i < functors.size(); ++i) {
    functors[i]();
  }
  callingPendingFunctors_ = false;
}
