#ifndef SJC_EVENTLOOPTHREADPOOL_H
#define SJC_EVENTLOOPTHREADPOOL_H

#include "Noncopyable.h"

#include <memory>
#include <vector>

namespace sjc
{

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool : noncopyable {
public:
  EventLoopThreadPool(EventLoop* baseLoop);
  ~EventLoopThreadPool();

  void setThreadNum(int numThreads) { numThreads_ = numThreads; }
  void start();
  EventLoop* getNextLoop();

private:
  EventLoop* baseLoop_; // 主线程
  bool started_;
  int numThreads_;
  int next_;
  std::vector<std::shared_ptr<EventLoopThread>> threads_; // 只有线程池持有对象的指针
  std::vector<EventLoop*> loops_; // EventLoop是线程中栈上的对象 不需要使用智能指针管理
  // 智能指针只用来管理堆上的对象 或者直接根据对象的创建方式 new or init
};

}

#endif // SJC_EVENTLOOPTHREADPOOL_H
