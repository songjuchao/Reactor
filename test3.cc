#include "EventLoop.h"
#include "Channel.h"

#include <iostream>
#include <chrono>

#include <strings.h>
#include <stdio.h>
#include <unistd.h> // close
#include <time.h> // cimte
#include <sys/timerfd.h> //

using std::chrono::system_clock;

sjc::EventLoop* g_loop;

void
timeout() {
  time_t now = system_clock::to_time_t(system_clock::now());
  std::cout << "TimeOut " << std::ctime(&now);

  g_loop->quit();
}

int
main() {
  time_t now = system_clock::to_time_t(system_clock::now());
  std::cout << "Started " << std::ctime(&now);
  sjc::EventLoop loop;
  g_loop = &loop;

  int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
  sjc::Channel channel(&loop, timerfd);
  channel.setReadCallback(timeout);
  channel.enableReading();

  struct itimerspec howlong;
  bzero(&howlong, sizeof howlong);
  howlong.it_value.tv_sec = 5;
  ::timerfd_settime(timerfd, 0, &howlong, NULL);

  loop.loop();

  ::close(timerfd);
}
