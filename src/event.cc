// Copyright 2016 Connor Taffe

#include "src/event.h"

#include <future>
#include <thread>
#include <vector>

namespace basilisk {

Event::~Event() {}

Publisher::~Publisher() {}

Subscriber::~Subscriber() {}

PublishSubscriber::PublishSubscriber() {}
PublishSubscriber::~PublishSubscriber() {}

Topic::~Topic() {}

void Topic::publish(std::shared_ptr<Event> e) {
  if (filter(e)) {
    for (auto h : handlers) {
      h->publish(e);
    }
  }
}

void Topic::subscribe(std::shared_ptr<Publisher> handler) {
  handlers.insert(handler);
}

void Topic::unsubscribe(std::shared_ptr<Publisher> handler) {
  handlers.erase(handler);
}

Spool *Spool::instance = nullptr;

Spool::Spool() {}

Spool *Spool::getInstance() {
  if (instance == nullptr) {
    instance = new Spool();
  }
  return instance;
}

// NOTE: assumes publish will in general be a heavy operation
// another approach might be pushing the entire loop into a detached thread,
// which would make publish() asynchronous.
void Spool::publish(std::shared_ptr<Event> e) {
  std::vector<std::future<void>> futures;
  for (auto h : handlers) {
    futures.push_back(std::async(std::launch::async, [=] { h->publish(e); }));
  }
}

void Spool::subscribe(std::shared_ptr<Publisher> handler) {
  handlers.insert(handler);
}

void Spool::unsubscribe(std::shared_ptr<Publisher> handler) {
  handlers.erase(handler);
}

}  // namespace basilisk
