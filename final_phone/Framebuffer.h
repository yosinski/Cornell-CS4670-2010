#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <QWidget>
#define __user
#include "omapfb.h"

class Framebuffer
{
  void * buffer;
  //struct fb_var_screeninfo var_info;
  struct fb_var_screeninfo overlay_info;
  struct omapfb_mem_info mem_info;
  struct omapfb_plane_info plane_info;
  int overlay_fd;  

  Framebuffer ();
  ~Framebuffer ();
  void SetOverlay (omapfb_color_format);

public:
  static Framebuffer & GetInstance ();
  void * GetBuffer ();
  void Enable (QWidget *);
  void Disable ();
  void SetColorKey (const QColor &);
  void SetYUV ();
  void SetGray ();
  void SetBGR ();
};

#endif
