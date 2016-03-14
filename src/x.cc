// Copyright 2016 Connor Taffe

#include "src/x.h"

#include <functional>
#include <iostream>
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
                [&](GLXContext *c) {
                  glXMakeCurrent(display.get(), None, nullptr);
                  glXDestroyContext(display.get(), *c);
                }};
  glXMakeCurrent(display.get(), *window.get(), *gl_context.get());
}

Window::Geometry Window::getGeometry() const {
  Geometry geometry;
  XGetGeometry(display.get(), *window.get(), geometry.root_window.get(),
               &geometry.xorigin, &geometry.yorigin, &geometry.width,
               &geometry.height, &geometry.border_width, &geometry.depth);
  return geometry;
}

std::tuple<int, int> Window::getPosition() const {
  auto geometry = getGeometry();
  return std::make_tuple(geometry.xorigin, geometry.yorigin);
}

void Window::setPosition(const std::tuple<int, int> &position) {
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
  XWindowChanges changes;
  changes.width = std::get<0>(dimensions);
  changes.height = std::get<1>(dimensions);
  XConfigureWindow(display.get(), *window.get(), CWWidth | CWHeight, &changes);
}

std::string Window::getTitle() const {
  char *buf;
  XFetchName(display.get(), *window.get(), &buf);
  return {buf};
}

void Window::setTitle(const std::string &title) {
  XStoreName(display.get(), *window.get(), title.c_str());
}

void Window::eventLoop() {
  std::unique_lock<std::mutex> lock(x_lock);
  // ICCCM communication
  const Atom kMsgWmDeleteWindow =
      XInternAtom(display.get(), "WM_DELETE_WINDOW", False);
  std::vector<Atom> msgs = {kMsgWmDeleteWindow};
  XSetWMProtocols(display.get(), *window.get(), msgs.data(),
                  static_cast<int>(msgs.size()));

  XEvent event;
  for (;;) {
    XNextEvent(display.get(), &event);
    switch (event.type) {
      case KeyPress:
        Spool::getInstance()->publish(
            std::shared_ptr<Event>(new x::event::Key(event.xkey)));
        break;
      case KeyRelease:
        Spool::getInstance()->publish(
            std::shared_ptr<Event>(new x::event::Key(event.xkey)));
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

Key::Key(XKeyEvent event) : xevent{event} {}

bool Key::isPress() const { return xevent.type == KeyPress; }

bool Key::isRelease() const { return xevent.type == KeyRelease; }

uint32_t Key::getCode() const { return xevent.keycode; }

std::string Key::description() { return "X key pressed/released event"; }

}  // namespace event
}  // namespace x
}  // namespace basilisk
