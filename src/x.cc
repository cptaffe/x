// Copyright 2016 Connor Taffe

#include "src/x.h"

#include <functional>
#include <iostream>
#include <map>
#include <vector>

namespace basilisk {

Window::~Window() {}

namespace window {

Event::~Event() {}

namespace event {

Key::~Key() {}

}  // namespace event
}  // namespace window

namespace x {

int Window::onError(Display *, XErrorEvent *error) {
  auto buf = std::array<char, 1024>();
  XGetErrorText(error->display, error->error_code, buf.data(), buf.size());
  std::cout << "X error: " << std::string(buf.data(), buf.size()) << std::endl;
  return 0;
}

Window::Window()
    : display{XOpenDisplay(nullptr), [](Display *d) { XCloseDisplay(d); }},
      screen{XDefaultScreen(display.get())} {
  XSetErrorHandler(onError);
  XSetWindowAttributes attributes;
  attributes.border_pixel = XBlackPixel(display.get(), screen);
  attributes.background_pixel = XWhitePixel(display.get(), screen);
  attributes.event_mask = KeyPressMask | KeyReleaseMask;
  window = {new ::Window(XCreateWindow(
                display.get(), XDefaultRootWindow(display.get()), 0, 0, 400,
                400, 0, CopyFromParent, CopyFromParent, CopyFromParent,
                CWBackPixel | CWBorderPixel | CWEventMask, &attributes)),
            [&](::Window *w) { XDestroyWindow(display.get(), *w); }};
  XMapWindow(display.get(), *window.get());
  int visual_attributes[] = {GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER,
                             None};
  gl_context = {new GLXContext(glXCreateContext(
                    display.get(),
                    glXChooseVisual(display.get(), screen, visual_attributes),
                    nullptr, true)),
                [&](GLXContext *c) { glXDestroyContext(display.get(), *c); }};
}

Window::Geometry Window::getGeometry() const {
  std::unique_lock<std::mutex> lock(x_lock);
  Geometry geometry;
  auto root = new ::Window;
  XGetGeometry(display.get(), *window.get(), root, &geometry.xorigin,
               &geometry.yorigin, &geometry.width, &geometry.height,
               &geometry.border_width, &geometry.depth);
  geometry.root_window = std::shared_ptr<::Window>{root};
  return geometry;
}

std::tuple<int, int> Window::getPosition() const {
  auto geometry = getGeometry();
  return std::make_tuple(geometry.xorigin, geometry.yorigin);
}

void Window::setPosition(const std::tuple<int, int> &position) {
  std::unique_lock<std::mutex> lock(x_lock);
  XWindowChanges changes;
  changes.x = std::get<0>(position);
  changes.y = std::get<1>(position);
  XConfigureWindow(display.get(), *window.get(), CWX | CWY, &changes);
}

std::tuple<int, int> Window::getDimensions() const {
  auto geometry = getGeometry();
  return std::make_tuple(geometry.width, geometry.height);
}

void Window::setDimensions(const std::tuple<int, int> &dimensions) {
  std::unique_lock<std::mutex> lock(x_lock);
  XWindowChanges changes;
  changes.width = std::get<0>(dimensions);
  changes.height = std::get<1>(dimensions);
  XConfigureWindow(display.get(), *window.get(), CWWidth | CWHeight, &changes);
}

std::string Window::getTitle() const {
  std::unique_lock<std::mutex> lock(x_lock);
  char *buf;
  XFetchName(display.get(), *window.get(), &buf);
  return {buf};
}

void Window::setTitle(const std::string &title) {
  std::unique_lock<std::mutex> lock(x_lock);
  XStoreName(display.get(), *window.get(), title.c_str());
}

void Window::bind() const {
  glXMakeCurrent(display.get(), *window.get(), *gl_context.get());
}

void Window::unbind() const { glXMakeCurrent(display.get(), None, nullptr); }

void Window::swap() { glXSwapBuffers(display.get(), *window.get()); }

void Window::eventLoop() {
  Atom kMsgWmDeleteWindow;
  {
    std::unique_lock<std::mutex> lock(x_lock);
    // ICCCM communication
    kMsgWmDeleteWindow = XInternAtom(display.get(), "WM_DELETE_WINDOW", False);
    std::vector<Atom> msgs = {kMsgWmDeleteWindow};
    XSetWMProtocols(display.get(), *window.get(), msgs.data(),
                    static_cast<int>(msgs.size()));
  }

  XEvent event;
  for (;;) {
    {
      std::unique_lock<std::mutex> lock(x_lock);
      XNextEvent(display.get(), &event);
    }
    switch (event.type) {
      case KeyPress:
        Spool::getInstance()->publish(
            std::shared_ptr<Event>(new x::event::Key{this, event.xkey}));
        break;
      case KeyRelease:
        Spool::getInstance()->publish(
            std::shared_ptr<Event>(new x::event::Key{this, event.xkey}));
        break;
      case ClientMessage:
        auto atom = static_cast<Atom>(event.xclient.data.l[0]);
        if (atom == kMsgWmDeleteWindow) {
          goto end_loop;
        }
        break;
    }
  }
end_loop : {
  // cleanup
}
}

std::thread Window::runEventLoop() {
  return std::thread(std::bind(&x::Window::eventLoop, this));
}

namespace event {

Key::Key(Window *parent, XKeyEvent event) : xevent{event}, window{parent} {}

namespace {

class CharacterCases {
 public:
  char lowercase, uppercase;
};

const auto dictionary = std::map<Key::Code, CharacterCases>{
    {Key::Code::kCharacterQ, {'q', 'Q'}},
    {Key::Key::Code::kCharacterW, {'w', 'W'}},
    {Key::Code::kCharacterE, {'e', 'E'}},
    {Key::Code::kCharacterR, {'r', 'R'}},
    {Key::Code::kCharacterT, {'t', 'T'}},
    {Key::Code::kCharacterY, {'y', 'Y'}},
    {Key::Code::kCharacterU, {'u', 'U'}},
    {Key::Code::kCharacterI, {'i', 'I'}},
    {Key::Code::kCharacterO, {'o', 'O'}},
    {Key::Code::kCharacterP, {'p', 'P'}},
    {Key::Code::kCharacterA, {'a', 'A'}},
    {Key::Code::kCharacterS, {'s', 'S'}},
    {Key::Code::kCharacterD, {'d', 'D'}},
    {Key::Code::kCharacterF, {'f', 'F'}},
    {Key::Code::kCharacterG, {'g', 'G'}},
    {Key::Code::kCharacterH, {'h', 'H'}},
    {Key::Code::kCharacterJ, {'j', 'J'}},
    {Key::Code::kCharacterK, {'k', 'K'}},
    {Key::Code::kCharacterL, {'l', 'L'}},
    {Key::Code::kCharacterZ, {'z', 'Z'}},
    {Key::Code::kCharacterX, {'x', 'X'}},
    {Key::Code::kCharacterC, {'c', 'C'}},
    {Key::Code::kCharacterV, {'v', 'V'}},
    {Key::Code::kCharacterB, {'b', 'B'}},
    {Key::Code::kCharacterN, {'n', 'N'}},
    {Key::Code::kCharacterM, {'m', 'M'}},
    {Key::Code::kNumber1, {'1', '!'}},
    {Key::Code::kNumber2, {'2', '@'}},
    {Key::Code::kNumber3, {'3', '#'}},
    {Key::Code::kNumber4, {'4', '$'}},
    {Key::Code::kNumber5, {'5', '%'}},
    {Key::Code::kNumber6, {'6', '^'}},
    {Key::Code::kNumber7, {'7', '&'}},
    {Key::Code::kNumber8, {'8', '*'}},
    {Key::Code::kNumber9, {'9', '('}},
    {Key::Code::kNumber0, {'0', ')'}},
    {Key::Code::kSpace, {' ', ' '}},
    {Key::Code::kDash, {'-', '_'}},
    {Key::Code::kEquals, {'=', '+'}},
    {Key::Code::kBackSpace, {'\b', '\b'}},
    {Key::Code::kTab, {'\t', '\t'}},
    {Key::Code::kLeftBracket, {'[', '{'}},
    {Key::Code::kRightBracket, {']', '}'}},
    {Key::Code::kReturn, {'\n', '\n'}},
    {Key::Code::kSemicolon, {';', ':'}},
    {Key::Code::kQuote, {'\'', '"'}},
    {Key::Code::kBackTick, {'`', '~'}},
    {Key::Code::kBackSlash, {'\\', '|'}},
    {Key::Code::kComma, {',', '<'}},
    {Key::Code::kPeriod, {'.', '>'}},
    {Key::Code::kForwardSlash, {'/', '?'}},
};

}  // namespace

std::string Key::description() { return "X key pressed/released event"; }

basilisk::Window *Key::getWindow() const { return window; }

bool Key::isPress() const { return xevent.type == KeyPress; }

bool Key::isRelease() const { return xevent.type == KeyRelease; }

bool Key::isShiftPressed() const { return (xevent.state & ShiftMask) != 0; }

bool Key::isControlPressed() const { return (xevent.state & ControlMask) != 0; }

bool Key::isCharacter() const {
  return dictionary.find(static_cast<Code>(xevent.keycode)) != dictionary.end();
}

char Key::toCharacter() const {
  auto c = dictionary.find(static_cast<Code>(xevent.keycode));
  if (c != dictionary.end()) {
    if (isShiftPressed()) {
      return c->second.uppercase;
    }
    return c->second.lowercase;
  }
  throw std::runtime_error(
      "Called .toCharacter() on non-character. Must check with "
      ".isCharacter() "
      "first");
}

std::tuple<int, int> Key::getCursorPosition() const {
  return std::make_tuple(xevent.x, xevent.y);
}

std::chrono::time_point<std::chrono::high_resolution_clock> Key::getTime()
    const {
  return std::chrono::time_point<std::chrono::high_resolution_clock>{
      std::chrono::milliseconds{xevent.time}};
}

Key::Code Key::getCode() const {
  return static_cast<Key::Code>(xevent.keycode);
}

}  // namespace event
}  // namespace x
}  // namespace basilisk
