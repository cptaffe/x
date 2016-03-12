// Copyright 2016 Connor Taffe

#include <GL/glx.h>
#include <X11/Xlib.h>

#include <array>
#include <cstdint>
#include <functional>
#include <iostream>
#include <memory>
#include <tuple>
#include <vector>

namespace x {

class Window {
 public:
  Window();
  virtual ~Window();

  std::tuple<int, int> getPosition() const;
  void setPosition(const std::tuple<int, int> &position);

  std::tuple<int, int> getDimensions() const;
  void setDimensions(const std::tuple<int, int> &position);

  std::string getTitle() const;
  void setTitle(const std::string &title);

  void dispatch();

 private:
  Display *display;
  int screen;
  ::Window window;
  GLXContext gl_context;

  class Geometry {
   public:
    ::Window root_window;
    int xorigin, yorigin;
    uint width, height, border_width, depth;
  };
  Geometry getGeometry() const;

  static int onError(Display *display, XErrorEvent *error);
};

}  // namespace x

int x::Window::onError(Display *, XErrorEvent *error) {
  auto buf = std::array<char, 1024>();
  XGetErrorText(error->display, error->error_code, buf.data(), buf.size());
  std::cout << "X error: " << std::string(buf.data(), buf.size()) << std::endl;
  return 0;
}

x::Window::Window()
    : display{XOpenDisplay(nullptr)}, screen{XDefaultScreen(display)} {
  XSetErrorHandler(onError);
  XSetWindowAttributes attributes;
  attributes.border_pixel = XBlackPixel(display, screen);
  attributes.background_pixel = XWhitePixel(display, screen);
  attributes.event_mask = KeyPressMask | KeyReleaseMask;
  window =
      XCreateWindow(display, XDefaultRootWindow(display), 0, 0, 400, 400, 0,
                    CopyFromParent, CopyFromParent, CopyFromParent,
                    CWBackPixel | CWBorderPixel | CWEventMask, &attributes);
  XMapWindow(display, window);
  int visual_attributes[] = {GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER,
                             None};
  gl_context = glXCreateContext(
      display, glXChooseVisual(display, screen, visual_attributes), nullptr,
      true);
  glXMakeCurrent(display, window, gl_context);
}

x::Window::~Window() {
  glXMakeCurrent(display, None, nullptr);
  glXDestroyContext(display, gl_context);
  XDestroyWindow(display, window);
  XCloseDisplay(display);
}

x::Window::Geometry x::Window::getGeometry() const {
  Geometry geometry;
  XGetGeometry(display, window, &geometry.root_window, &geometry.xorigin,
               &geometry.yorigin, &geometry.width, &geometry.height,
               &geometry.border_width, &geometry.depth);
  return geometry;
}

std::tuple<int, int> x::Window::getPosition() const {
  auto geometry = getGeometry();
  return std::make_tuple(geometry.xorigin, geometry.yorigin);
}

void x::Window::setPosition(const std::tuple<int, int> &position) {
  XWindowChanges changes;
  changes.x = std::get<0>(position);
  changes.y = std::get<1>(position);
  XConfigureWindow(display, window, CWX | CWY, &changes);
}

std::tuple<int, int> x::Window::getDimensions() const {
  auto geometry = getGeometry();
  return std::make_tuple(geometry.width, geometry.height);
}

void x::Window::setDimensions(const std::tuple<int, int> &dimensions) {
  XWindowChanges changes;
  changes.width = std::get<0>(dimensions);
  changes.height = std::get<1>(dimensions);
  XConfigureWindow(display, window, CWWidth | CWHeight, &changes);
}

std::string x::Window::getTitle() const {
  char *buf;
  XFetchName(display, window, &buf);
  return {buf};
}

void x::Window::setTitle(const std::string &title) {
  XStoreName(display, window, title.c_str());
}

void x::Window::dispatch() {
  // ICCCM communication
  const Atom kMsgWmDeleteWindow =
      XInternAtom(display, "WM_DELETE_WINDOW", False);
  std::vector<Atom> msgs = {kMsgWmDeleteWindow};
  XSetWMProtocols(display, window, msgs.data(), static_cast<int>(msgs.size()));

  XEvent event;
  for (;;) {
    XNextEvent(display, &event);
    switch (event.type) {
      case KeyPress:
        goto end_loop;
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

int main() {
  x::Window window;
  window.setDimensions(std::make_tuple(500, 600));
  window.setTitle("basilisk");
  window.dispatch();
}
