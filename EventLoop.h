#ifndef SJC_EVENTLOOP_H
#define SJC_EVENTLOOP_H

#include "Noncopyable.h"

#include <vector>
#include <memory>
#include <functional>
#include <thread>
#include <mutex> // std::mutex std::lock_guard
#include <atomic> // std::atmoic

namespace sjc
{

class Channel;
class Poller;
// class TimerQueue;

class EventLoop : public noncopyable {
public:
  typedef std::function<void()> Functor;

  EventLoop();
  ~EventLoop();

  void loop(); // 只能在创建EventLoop对象的线程中调用
  void quit(); // 可以跨线程调用

  // 确保线程安全 可以跨线程调用
  // 当在本线程调用函数时 直接执行cb()
  // 否则会调用queueInLoop()函数
  void runInLoop(const Functor& cb);
  void queueInLoop(const Functor& cb);

  void updateChannel(Channel* channel);
  void removeChannel(Channel* channel);

  void assertInLoopThread() {
    if (!isInLoopThread()) {
      abortNotInLoopThread();
    }
  }
bool isInLoopThread() const { return tid_ == std::this_thread::get_id(); }

private:
  void abortNotInLoopThread();
  void handleRead(); // wakeupFd_回调函数
  void wakeup();
  void doPendingFunctors();

private:
  typedef std::vector<Channel*> ChannelList;

  // std::atomic<bool> looping_;
  std::atomic<bool> quit_; // 不同线程访问时不会导致数据竞争
  // 用标志位作为退出判断是为了退出之前可以继续处理已经触发过的activeChannels事件
  // 需要使用原子类型 因为quit函数可以跨线程调用 可能有多个线程访问
  std::atomic<bool> callingPendingFunctors_; // 是否在执行用户回调 需要原子型
  std::thread::id tid_;
  std::unique_ptr<Poller> poller_;
  int wakeupFd_; // 唤醒EventLoop线程的fd 唤醒时向该线程写入即可
  std::unique_ptr<Channel> wakeupChannel_; // 负责wakeupFd_
  // 生命周期完全由EventLoop对象掌控
  ChannelList activeChannels_;
  std::mutex mutex_; // 锁是为了保护pendingFuncors
  // 使用该变量的queueInLoop函数可以跨线程调用 这里没办法使用std::atomic类型
  // 标准库里的容器都不是线程安全的
  std::vector<Functor> pendingFuncors; // 用户直接设置的回调函数
};

} // sjc

#endif // SJC_EVENTLOOP_H
