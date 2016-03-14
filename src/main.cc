// Copyright 2016 Connor Taffe

#include "src/event.h"
#include "src/x.h"

#include <future>
#include <vector>

int main() {
  std::vector<std::future<void>> futures;
  for (auto i = 0; i < 4; i++) {
    futures.push_back(std::async(std::launch::async, [] {
      basilisk::x::Window window;
      window.setDimensions(std::make_tuple(500, 600));
      window.setTitle("basilisk");
      auto t = window.runEventLoop();
      t.join();
    }));
  }
}
