#include "Viewfinder.h"
#include <iostream>
#include "CameraThread.h"

Viewfinder::Viewfinder ( QWidget * parent )
  : QWidget ( parent )
{
  QWidget::setBackgroundRole ( QPalette::Window );
  QWidget::setAutoFillBackground ( true );
  QPalette overlayPalette = QWidget::palette ();
  overlayPalette.setColor ( QPalette::Window, colorKey () );
  QWidget::setPalette ( overlayPalette );

  if ( -1 == ( overlay_fd = open ( "/dev/fb1", O_RDWR ) ) )
  {
    emit alert ( "Failed to open framebuffer" );
  }

  if ( ioctl ( overlay_fd, FBIOGET_VSCREENINFO, &overlay_info ) )
  {
    emit alert ( "FBIO_VSCREENINFO failed" );
  }

  if ( ioctl ( overlay_fd, OMAPFB_QUERY_PLANE, &plane_info ) )
  {
    emit alert ( "OMAPGB_QUERY_PLANE failed" );
  }

  plane_info.enabled = 0;
  plane_info.pos_x = 0;
  plane_info.pos_y = 0;
  plane_info.out_width = 800;
  plane_info.out_height = 480;

  if ( ioctl ( overlay_fd, OMAPFB_SETUP_PLANE, &plane_info ) )
  {
    emit alert ( "OMAPFB_SETUP_PLANE failed" );
  }

  mem_info.size = 800 * 480 * 2;
  mem_info.type = 0;

  if ( ioctl ( overlay_fd, OMAPFB_SETUP_MEM, &mem_info ) )
  {
    emit alert ( "OMAPFB_SETUP_MEM failed" );
  }

  void * ptr = mmap ( NULL,
                      mem_info.size,
                      PROT_WRITE,
                      MAP_SHARED,
                      overlay_fd,
                      0 );
  if ( ptr == MAP_FAILED )
  {
    emit alert ( "mmap failed" );
  }

  framebuffer_ = FCam::Image ( 800, 480, FCam::UYVY, ( unsigned char * )ptr );

  memset ( ptr, 128, 800 * 480 * 2 );

  overlay_info.xres = 800;
  overlay_info.yres = 480;
  overlay_info.xres_virtual = 800;
  overlay_info.yres_virtual = 480;
  overlay_info.xoffset = 0;
  overlay_info.yoffset = 0;
  overlay_info.nonstd = OMAPFB_COLOR_YUV422;

  if ( ioctl ( overlay_fd, FBIOPUT_VSCREENINFO, &overlay_info ) )
  {
    emit alert ( "FBIOPUT_VSCREENINFO failed" );
  }

  struct omapfb_color_key color_key;
  color_key.key_type = OMAPFB_COLOR_KEY_GFX_DST;
  QColor key = colorKey ();
  color_key.trans_key =
    ( ( key.red () >>
        3 ) << 11 ) | ( ( key.green () >> 2 ) << 5 ) | ( ( key.blue () >> 3 ) );
  if ( ioctl ( overlay_fd, OMAPFB_SET_COLOR_KEY, &color_key ) )
  {
    emit alert ( "OMAPFB_SET_COLOR_KEY failed" );
  }

  filterInstalled = false;

  CameraThread::getInstance ().setFramebuffer ( framebuffer () );
  CameraThread::getInstance ().start ();
}

bool Viewfinder::eventFilter ( QObject *, QEvent * event )
{
  if ( event->type () == QEvent::Move ||
       event->type () == QEvent::Resize ||
       event->type () == QEvent::Show )
  {
    enable ();
  }
  else if ( event->type () == QEvent::Hide )
  {
    disable ();
  }

  // We don't capture this event, it should be propagated as normal
  return false;
}

void Viewfinder::showEvent ( QShowEvent * )
{
  enable ();
}

void Viewfinder::hideEvent ( QHideEvent * )
{
  disable ();
}

void Viewfinder::resizeEvent ( QResizeEvent * )
{
  enable ();
}

void Viewfinder::moveEvent ( QMoveEvent * )
{
  enable ();
}


Viewfinder::~Viewfinder()
{
  disable ();
  ::close ( overlay_fd );
}

void Viewfinder::enable ()
{
  std::cerr << "Enable" << std::endl;
  // Shift the plane according to where the widget is, but keep it
  // at 800x480

  QPoint global = mapToGlobal ( QPoint ( 0, 0 ) );

  // round to even X
  global.setX ( global.x () / 2 );
  global.setX ( global.x () * 2 );
  int xoff = global.x () > 0 ? global.x () : 0;
  int yoff = global.y () > 0 ? global.y () : 0;
  int xcrop = global.x () < 0 ? -global.x () : 0;
  int ycrop = global.y () < 0 ? -global.y () : 0;

  if ( xcrop > 800 || ycrop > 480 )
  {
    disable ();
    return;
  }

  // Set the size and position on screen
  plane_info.enabled = 1;
  plane_info.pos_x = xoff;
  plane_info.pos_y = yoff;
  plane_info.out_width = 800 - xcrop;
  plane_info.out_height = 480 - ycrop - yoff;

  if ( ioctl ( overlay_fd, OMAPFB_SETUP_PLANE, &plane_info ) )
  {
    emit alert ( "OMAPFB_SETUP_PLANE failed" );
  }

  // The image is always 800x480
  overlay_info.xres_virtual = 800;
  overlay_info.yres_virtual = 480 - yoff;
  // Set the portion of it that's visible on screen
  overlay_info.xres = plane_info.out_width;
  overlay_info.yres = plane_info.out_height;
  overlay_info.xoffset = xcrop;
  overlay_info.yoffset = ycrop;
  overlay_info.nonstd = OMAPFB_COLOR_YUV422;
  if ( ioctl ( overlay_fd, FBIOPUT_VSCREENINFO, &overlay_info ) )
  {
    emit alert ( "FBIOPUT_VSCREENINFO failed" );
  }

  if ( !filterInstalled )
  {
    // if anything moves above me, we need to know about it to update the overlay
    for ( QObject * obj = parent (); obj; obj = obj->parent () )
    {
      obj->installEventFilter ( this );
    }
    filterInstalled = true;
  }
}

void Viewfinder::disable ()
{
  return;
  // I don't know what the deal is and I'm tired.
  std::cerr << "Disable" << std::endl;
  plane_info.enabled = 0;
  if ( ioctl ( overlay_fd, OMAPFB_SETUP_PLANE, &plane_info ) )
  {
    emit alert ( "OMAPFB_SETUP_PLANE failed" );
  }
}

FCam::Image Viewfinder::framebuffer ()
{
  return framebuffer_;
}
