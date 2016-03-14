// Copyright 2016 Connor Taffe

#ifndef SRC_X_H_
#define SRC_X_H_

#include <GL/glx.h>
#include <X11/Xlib.h>

#include <chrono>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <tuple>

#include "src/event.h"

namespace basilisk {

// Window interface
class Window {
 public:
  virtual ~Window();

  virtual std::tuple<int, int> getPosition() const = 0;
  virtual void setPosition(const std::tuple<int, int> &position) = 0;

  virtual std::tuple<int, int> getDimensions() const = 0;
  virtual void setDimensions(const std::tuple<int, int> &dimensions) = 0;

  virtual std::string getTitle() const = 0;
  virtual void setTitle(const std::string &title) = 0;
};

namespace window {

// Window events topic
// Filters events to relay only Window events
class Topic : public basilisk::Topic {
 public:
  virtual ~Topic();

 protected:
  bool filter(std::shared_ptr<Event> e) final;
};

// Window events
// All window events inherit from this event
class Event : public basilisk::Event {
 public:
  virtual ~Event();
  virtual Window *getWindow() const = 0;
};

namespace event {

// Keypress event
class Key : public Event {
 public:
  virtual ~Key();
  virtual bool isPress() const = 0;
  virtual bool isRelease() const = 0;
  virtual uint32_t getCode() const = 0;
};

// Cursor movement event
class Movement : public Event {
 public:
  virtual ~Movement();
  virtual std::chrono::duration<double> getTime() const = 0;
  virtual std::tuple<int, int> getCoordinates() const = 0;
};

}  // namespace event
}  // namespace window

namespace x {

// x Window specialization
class Window : public basilisk::Window {
 public:
  Window();

  std::tuple<int, int> getPosition() const override;
  void setPosition(const std::tuple<int, int> &position) override;

  std::tuple<int, int> getDimensions() const override;
  void setDimensions(const std::tuple<int, int> &dimensions) override;

  std::string getTitle() const override;
  void setTitle(const std::string &title) override;

  // Spawns events
  std::thread runEventLoop();

 private:
  // explicit lock instead of using XInitThreads
  mutable std::mutex x_lock;
  std::shared_ptr<Display> display;
  int screen;
  std::shared_ptr<::Window> window;
  std::shared_ptr<GLXContext> gl_context;

  class Geometry {
   public:
    std::shared_ptr<::Window> root_window;
    int xorigin, yorigin;
    uint width, height, border_width, depth;
  };
  Geometry getGeometry() const;

  static int onError(Display *display, XErrorEvent *error);
  void eventLoop();
};

namespace event {

// Keypress event
class Key : public window::event::Key {
 public:
  Key(Window *parent, XKeyEvent event);
  bool isPress() const override;
  bool isRelease() const override;
  uint32_t getCode() const override;
  basilisk::Window *getWindow() const override;
  std::string description() override;

 private:
  const XKeyEvent xevent;
  Window *window;
};

}  // namespace event
}  // namespace x
}  // namespace basilisk

#endif  // SRC_X_H_
