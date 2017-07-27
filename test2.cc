#include "EventLoop.h"

#include <thread>

sjc::EventLoop* g_loop;

void
threadFunc() {
	g_loop->loop();
}

int
main() {
	sjc::EventLoop loop;
	g_loop = &loop;

	std::thread t(threadFunc);
	t.join();
}
