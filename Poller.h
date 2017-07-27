#ifndef SJC_POLLER_H
#define SJC_POLLER_H

#include <map>
#include <vector>

#include "EventLoop.h"
#include "Noncopyable.h"

struct pollfd;

namespace sjc
{

class Channel;

class Poller : public noncopyable {
public:
  typedef std::vector<Channel*> ChannelList;

  Poller(EventLoop* loop);
  ~Poller();

  void poll(int timeOutMs, ChannelList* activeChannels);

  void updateChannel(Channel* channel);
  void removeChannel(Channel* channel);

  void assertInLoopThread() { owerLoop_->assertInLoopThread(); }

private:
  void fillActiveChannels(int numEvents, ChannelList* activeChannels) const;

  typedef std::vector<struct pollfd> PollFdList;
  typedef std::map<int, Channel*> ChannelMap; // 将fd Channel -> map

  EventLoop* owerLoop_; // 持有该类的EventLoop对象
  PollFdList pollfds_;
  ChannelMap channels_;
};

}

#endif // SJC_POLLER_H
