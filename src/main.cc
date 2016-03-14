// Copyright 2016 Connor Taffe

#include <future>
#include <iostream>
#include <vector>

#include "src/event.h"
#include "src/x.h"

class Publisher : public basilisk::Publisher {
  void publish(std::shared_ptr<basilisk::Event> event) override;
};

void Publisher::publish(std::shared_ptr<basilisk::Event> event) {
  ([&](std::shared_ptr<basilisk::window::event::Key> kevent) {
    if (kevent != nullptr) {
      if (kevent->isPress()) {
        if (kevent->isCharacter()) {
          std::cout << kevent->toCharacter() << std::flush;
        }
      }
    }
    std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(
                     kevent->getTime().time_since_epoch())
                     .count()
              << std::endl;
  })(std::dynamic_pointer_cast<basilisk::window::event::Key>(event));
}

int main() {
  std::vector<std::future<void>> futures;
  for (auto i = 0; i < 1; i++) {
    futures.push_back(std::async(std::launch::async, [] {
      basilisk::x::Window window;
      window.setDimensions(std::make_tuple(500, 600));
      window.setTitle("basilisk");
      auto t = window.runEventLoop();
      t.join();
    }));
  }
  basilisk::Spool::getInstance()->subscribe(
      std::shared_ptr<Publisher>(new Publisher{}));
}
