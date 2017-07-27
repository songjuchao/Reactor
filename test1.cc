#include "EventLoop.h"

#include <thread>
#include <iostream>

#include <sys/types.h>
#include <unistd.h>

void
threadFunc() {
	printf("threadFunc() : pid = %d, tid = ", getpid());
	std::cout << std::this_thread::get_id() << "\n";

	sjc::EventLoop loop;
	loop.loop();
}

int
main() {
	printf("main() : pid = %d, tid = ", getpid());
	std::cout << std::this_thread::get_id() << "\n";
	sjc::EventLoop loop;

	std::thread t(threadFunc);
	// t.join();

	loop.loop();
}
