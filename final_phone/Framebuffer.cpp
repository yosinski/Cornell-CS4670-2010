#include "Framebuffer.h"
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

Framebuffer & Framebuffer::GetInstance ()
{
  static Framebuffer framebuffer;
  return framebuffer;
}

Framebuffer::Framebuffer ()
{
  // Open the overlay device
  overlay_fd = open("/dev/fb1", O_RDWR);

  if (overlay_fd == -1) 
  {
    perror("open");
  }


  if (ioctl(overlay_fd, OMAPFB_QUERY_PLANE, &plane_info)) 
  {
    perror("OMAPFB_QUERY_PLANE");
  }

  // Disable the plane so we can allocate memory for it. 
  plane_info.enabled = 0;
  plane_info.pos_x = 0; 
  plane_info.pos_y = 0; 
  plane_info.out_width = 640;
  plane_info.out_height = 480;
  if (ioctl(overlay_fd, OMAPFB_SETUP_PLANE, &plane_info)) 
  {
    perror("OMAPFB_SETUP_PLANE");
  }

  // Allocate the memory
  mem_info.size = 640*480*4;
  mem_info.type = 0;
  if (ioctl(overlay_fd, OMAPFB_SETUP_MEM, &mem_info)) 
  {
    perror("OMAPFrgb24p formatB_SETUP_MEM");
  }

  // mmap it into an FCam image
  buffer = mmap(NULL, mem_info.size, PROT_WRITE, MAP_SHARED, overlay_fd, 0);
  if (buffer == MAP_FAILED)
  {
    perror("mmap");
  }

  // Clear the memory in case there was something hanging around from an 
  // earlier invocation
  memset(buffer, 128, 640*480*4);

  // Set up the color key
  struct omapfb_color_key color_key;
  color_key.key_type = OMAPFB_COLOR_KEY_GFX_DST;
  SetColorKey (QColor (10, 0, 10));
}

void Framebuffer::SetOverlay (omapfb_color_format colorMap)
{
    // Get the current overlay and plane settings
    if (ioctl(overlay_fd, FBIOGET_VSCREENINFO, &overlay_info))
    {
      perror("FBIO_VSCREENINFO");
    }

  // Set the overlay properties
  overlay_info.xres = 640;
  overlay_info.yres = 480;
  overlay_info.xres_virtual = 640;
  overlay_info.yres_virtual = 480;
  overlay_info.xoffset = 0;
  overlay_info.yoffset = 0;
  overlay_info.nonstd = colorMap;
  if (ioctl(overlay_fd, FBIOPUT_VSCREENINFO, &overlay_info))
  {
    perror("FBIOPUT_VSCREENINFO");
  }
}

void Framebuffer::SetBGR ()
{
  SetOverlay (OMAPFB_COLOR_YUV422);
}

void Framebuffer::SetGray ()
{
  SetOverlay (OMAPFB_COLOR_YUV422);
}

void Framebuffer::SetYUV ()
{
  SetOverlay (OMAPFB_COLOR_YUV422);
}

void Framebuffer::SetColorKey (const QColor & color)
{
  // Set up the color key
  struct omapfb_color_key color_key;
  color_key.key_type = OMAPFB_COLOR_KEY_GFX_DST;
  color_key.trans_key = ((color.red() >> 3) << 11) | ((color.green() >> 2) << 5)                        
                        | ((color.blue() >> 3));
  if (ioctl(overlay_fd, OMAPFB_SET_COLOR_KEY, &color_key))
  {
    perror("OMAPFB_SET_COLOR_KEY");
  }
}

Framebuffer::~Framebuffer ()
{
  Disable ();
  close (overlay_fd);
}

void Framebuffer::Enable (QWidget * widget)
{
  // Shift the plane according to where the widget is, but keep it
  // at 640x480

  QPoint global = widget->mapToGlobal(QPoint(0, 0));

  // round to even X
  global.setX(global.x()/2);
  global.setX(global.x()*2);

  int xoff = global.x() > 0 ? global.x() : 0;
  int yoff = global.y() > 0 ? global.y() : 0;
  int xcrop = global.x() < 0 ? -global.x() : 0;
  int ycrop = global.y() < 0 ? -global.y() : 0;

  if (xcrop > 640 || ycrop > 480) 
  {
    Disable();
    return;
  }

  if (ioctl(overlay_fd, OMAPFB_QUERY_PLANE, &plane_info))
  {
    perror("OMAPFB_QUERY_PLANE");
  }

  // Set the size and position on screen
  plane_info.enabled = 1;
  plane_info.pos_x = xoff;
  plane_info.pos_y = yoff;
  plane_info.out_width = 640 - xcrop;
  plane_info.out_height = 480 - ycrop;

  if (ioctl(overlay_fd, OMAPFB_SETUP_PLANE, &plane_info)) 
  {
    perror("OMAPFB_SETUP_PLANE");
  }

  // The image is always 640x480
//  overlay_info.xres_virtual = 640;
 // overlay_info.yres_virtual = 480;
  // Set the portion of it that's visible on screen
//  overlay_info.xres = plane_info.out_width;
//  overlay_info.yres = plane_info.out_height;
//  overlay_info.xoffset = xcrop;
//  overlay_info.yoffset = ycrop;
//  overlay_info.nonstd = OMAPFB_COLOR_YUV422;
//  if (ioctl(overlay_fd, FBIOPUT_VSCREENINFO, &overlay_info))
//  {
//    perror("FBIOPUT_VSCREENINFO");
//  }
}

void Framebuffer::Disable ()
{
 plane_info.enabled = 0;
 if (ioctl(overlay_fd, OMAPFB_SETUP_PLANE, &plane_info)) 
 {
   perror("OMAPFB_SETUP_PLANE");
 }
}

void * Framebuffer::GetBuffer ()
{
  return buffer;
}
