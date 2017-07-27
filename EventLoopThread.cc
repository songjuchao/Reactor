#include "EventLoopThread.h"
#include "EventLoop.h"

using namespace sjc;

EventLoopThread::EventLoopThread()
  : loop_(NULL) {
}

EventLoopThread::~EventLoopThread() { }

/*
EventLoop*
EventLoopThread::startLoop() {
  thread_.reset(new std::thread(&EventLoopThread::threadFunc, this));
  return loop_;
}
*/
// 上述这种写法是错误的
// 因为std::thread可能还没有启动 这时返回的loop_仍然为空
EventLoop*
EventLoopThread::startLoop() {
  thread_ =  std::move(std::thread(std::bind(&EventLoopThread::threadFunc, this)));

  std::unique_lock<std::mutex> lk(mutex_); // 可以随时释放锁 语义更加准确
  // std::lock_guard语义是构造时加锁 析构时释放锁 语义不够准确
  while (loop_ == NULL)
    cv_.wait(lk);
  return loop_;
}

void
EventLoopThread::threadFunc(void) {
  std::lock_guard<std::mutex> lk(mutex_);
  EventLoop loop;
  loop_ = &loop;
  cv_.notify_one();
  loop.loop();
}
