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

  virtual void bind() const = 0;
  virtual void unbind() const = 0;
  virtual void swap() = 0;
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
  virtual bool isShiftPressed() const = 0;
  virtual bool isControlPressed() const = 0;

  enum class Code : uint32_t {
    kEsc = 9,
    kNumber1,
    kNumber2,
    kNumber3,
    kNumber4,
    kNumber5,
    kNumber6,
    kNumber7,
    kNumber8,
    kNumber9,
    kNumber0,
    kDash,
    kEquals,
    kBackSpace,
    kTab,
    kCharacterQ,
    kCharacterW,
    kCharacterE,
    kCharacterR,
    kCharacterT,
    kCharacterY,
    kCharacterU,
    kCharacterI,
    kCharacterO,
    kCharacterP,
    kLeftBracket,
    kRightBracket,
    kReturn,
    kLeftControl,
    kCharacterA,
    kCharacterS,
    kCharacterD,
    kCharacterF,
    kCharacterG,
    kCharacterH,
    kCharacterJ,
    kCharacterK,
    kCharacterL,
    kSemicolon,
    kQuote,
    kBackTick,
    kLeftShift,
    kBackSlash,
    kCharacterZ,
    kCharacterX,
    kCharacterC,
    kCharacterV,
    kCharacterB,
    kCharacterN,
    kCharacterM,
    kComma,
    kPeriod,
    kForwardSlash,
    kRightShift,
    kAsterisk,
    kLeftAlternate,
    kSpace,
    kCapsLock,
    kUp = 111,
    kDown = 116,
    kLeft = 113,
    kRight = 114,
    kFunction1 = 67,
    kFunction2,
    kFunction3,
    kFunction4,
    kFunction5,
    kFunction6,
    kFunction7,
    kFunction8,
    kFunction9,
    kFunction10,
    kFunction11 = 95,
    kFunction12,
  };
  virtual Code getCode() const = 0;

  virtual bool isCharacter() const = 0;
  virtual char toCharacter() const = 0;

  virtual std::tuple<int, int> getCursorPosition() const = 0;
  virtual std::chrono::time_point<std::chrono::high_resolution_clock> getTime()
      const = 0;
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

  void bind() const override;
  void unbind() const override;

  void swap() override;

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
  std::string description() override;
  basilisk::Window *getWindow() const override;
  bool isPress() const override;
  bool isRelease() const override;
  bool isShiftPressed() const override;
  bool isControlPressed() const override;
  bool isCharacter() const override;
  char toCharacter() const override;
  Code getCode() const override;
  std::tuple<int, int> getCursorPosition() const override;
  std::chrono::time_point<std::chrono::high_resolution_clock> getTime()
      const override;

 private:
  const XKeyEvent xevent;
  Window *window;
};

}  // namespace event
}  // namespace x
}  // namespace basilisk

#endif  // SRC_X_H_
