#pragma once

namespace display {
void init();
void dispose();
void poll();

struct Window {
  virtual int   getX()         = 0;
  virtual int   getY()         = 0;
  virtual int   getWidth()     = 0;
  virtual int   getHeight()    = 0;
  virtual void  swapBuffers()  = 0;
  virtual bool  shouldClose()  = 0;
  virtual float getDeltaTime() = 0;
  virtual ~Window() {}
};

Window* windowCreate();
void    windowMakeCurrent(Window*);
} // namespace display
