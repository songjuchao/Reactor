#ifndef SJC_EVENTLOOPTHREAD_H
#define SJC_EVENTLOOPTHREAD_H

#include "Noncopyable.h"

#include <thread>
#include <functional>
#include <memory>
#include <condition_variable>

namespace sjc
{

class EventLoop;

class EventLoopThread : noncopyable {
public:
	EventLoopThread();
	~EventLoopThread();

	EventLoop* startLoop();

private:
	void threadFunc();

private:
	EventLoop* loop_;
	std::thread thread_;
	std::mutex mutex_;
	std::condition_variable cv_;
};

}

#endif
