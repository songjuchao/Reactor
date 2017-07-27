#include "EventLoopThreadPool.h"
#include "EventLoop.h"
#include "EventLoopThread.h"

#include <assert.h>

using namespace sjc;

EventLoopThreadPool::EventLoopThreadPool(EventLoop* baseLoop)
  : baseLoop_(baseLoop),
    started_(false),
    numThreads_(0),
    next_(0) {
}

EventLoopThreadPool::~EventLoopThreadPool() {
  for (auto loop : loops_)
    loop->quit();
}

void
EventLoopThreadPool::start() {
  assert(!started_);
  baseLoop_->assertInLoopThread();
  started_ = true;

  for (int i = 0; i < numThreads_; ++i) {
    auto t = std::make_shared<EventLoopThread>();
    threads_.push_back(t);
    loops_.push_back(t->startLoop());
  }
}

EventLoop*
EventLoopThreadPool::getNextLoop() {
  baseLoop_->assertInLoopThread();
  EventLoop* loop = baseLoop_; // 只有主线程时返回主线程

  if (!loops_.empty()) {
    // round-robin
    loop = loops_[next_];
    ++next_;
    if (static_cast<size_t>(next_) >= loops_.size()) {
      next_ = 0;
    }
  }

  return loop;
}
