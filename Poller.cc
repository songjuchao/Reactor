#include "Poller.h"
#include "Channel.h"

#include <iostream>

#include <assert.h>
#include <poll.h>

using namespace sjc;

Poller::Poller(EventLoop* loop)
  : owerLoop_(loop) {
}

Poller::~Poller() { }

void
Poller::poll(int timeOutMs, ChannelList* activeChannels) {
  int numEvents = ::poll(&*pollfds_.begin(), pollfds_.size(), timeOutMs);
  if (numEvents > 0) {
    fillActiveChannels(numEvents, activeChannels);
  } else if (numEvents == 0) {
    std::clog << "Poller::poll() timeout nothing happend\n";
  } else {
    std::cerr << "Poller::poll()\n";
  }
}

void
Poller::fillActiveChannels(int numEvents, ChannelList* activeChannels) const {
  for (auto pfd_iter = pollfds_.begin(); pfd_iter != pollfds_.end()
       && numEvents > 0; ++pfd_iter) {
      if (pfd_iter->revents > 0) {
        --numEvents;
        auto ch_iter = channels_.find(pfd_iter->fd);
        assert(ch_iter != channels_.end());
        auto channel = ch_iter->second;
        assert(pfd_iter->fd == channel->fd());
        channel->set_revents(pfd_iter->revents);
        activeChannels->push_back(channel);
      }
  }
}

void
Poller::updateChannel(Channel* channel) {
  assertInLoopThread();
  // std::clog << "fd = " << channel->fd() << " events = " << channel->events();
  if (channel->index() < 0) { // new Channel
    assert(channels_.find(channel->fd()) == channels_.end());
    struct pollfd pfd;
    pfd.fd = channel->fd();
    pfd.events = static_cast<short>(channel->events());
    pfd.revents = 0;
    pollfds_.push_back(pfd);
    int idx = static_cast<int>(pollfds_.size() - 1);
    channel->set_index(idx);
    channels_[pfd.fd] = channel;
  } else { // update existing Channel
    assert(channels_.find(channel->fd()) != channels_.end());
    assert(channels_[channel->fd()] == channel);
    int idx = channel->index();
    assert(0 <= idx && idx < static_cast<int>(pollfds_.size())); //

    auto& pfd = pollfds_[idx];
    assert(pfd.fd == channel->fd() || pfd.fd == -channel->fd() - 1);
    pfd.events = static_cast<short>(channel->events()); //
    pfd.revents = 0;
    if (channel->isNoneEvent()) {
      // ignore this pollfd
      pfd.fd = -channel->fd() - 1;
    }
  }
}

void
Poller::removeChannel(Channel* channel) {
  assertInLoopThread();
  // std::clog << "fd = " << channel->fd();
  assert(channels_.find(channel->fd()) != channels_.end());
  assert(channels_[channel->fd()] == channel);
  assert(channel->isNoneEvent()); //
  int idx = channel->index();
  assert(0 <= idx && idx <= static_cast<int>(pollfds_.size()));

  const auto& pfd = pollfds_[idx];
  assert(pfd.fd == -channel->fd() - 1 && pfd.events == channel->events()); //

  size_t n = channels_.erase(channel->fd());
  assert(n == 1);

  if (static_cast<size_t>(idx) == pollfds_.size() - 1) {
    pollfds_.pop_back();
  } else {
    int channelAtEnd = pollfds_.back().fd;
    iter_swap(pollfds_.begin() + idx, pollfds_.end() - 1); // 交换两个元素
    if (channelAtEnd < 0) { // 如果pollfds_的最后一个元素恰好是之前不关心的fd
      channelAtEnd = -channelAtEnd - 1; // 得到正确的fd
    }
    channels_[channelAtEnd]->set_index(idx);
    pollfds_.pop_back();
  }
}
