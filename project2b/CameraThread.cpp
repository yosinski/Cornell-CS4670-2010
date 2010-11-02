/** \file */

#include <qmetatype.h>
#include "CameraThread.h"

#include <FCam/N900.h>

#include <vector>
#include <iostream>

#include "Viewfinder.h"

using namespace std;

namespace Plat = FCam::N900;

CameraThread & CameraThread::getInstance ()
{
  static CameraThread cameraThread;

  return cameraThread;
}

void CameraThread::setFramebuffer ( FCam::Image framebuffer )
{
  overlay = framebuffer;
}

void CameraThread::run ()
{
  // Make an asynchronous file writer to save images in the background
  FCam::AsyncFileWriter writer;
  Plat::Sensor sensor;
  Plat::Lens lens;
  Plat::Flash flash;

  // tell the sensor that the flash and the lens will be tagging
  // frames that come back from it
  sensor.attach ( &flash );
  sensor.attach ( &lens );

  // Make a helper autofocus object
  FCam::AutoFocus autoFocus ( &lens );

  // The viewfinder shot
  FCam::Shot viewfinder;
  viewfinder.exposure = 40000;
  viewfinder.gain = 1.0f;
  // run at 25 fps
  viewfinder.frameTime = 40000;
  // dump image data directly into the frame buffer
  viewfinder.image = overlay;
  // enable histograms and sharpness maps
  viewfinder.histogram.enabled = true;
  viewfinder.histogram.region = FCam::Rect ( 0, 0, 640, 480 );
  viewfinder.sharpness.enabled = true;
  viewfinder.sharpness.size = FCam::Size ( 16, 12 );

  // A full 5MP photograph. We'll set the exposure, frameTime, and
  // gain later, after we meter. Default parameters apply (no
  // histograms or sharpness), image memory auto allocated for each
  // new photograph, so that we can have multiple unique photographs
  // saving at once.
  FCam::Shot photo;
  photo.image = FCam::Image ( 480, 360, FCam::UYVY, FCam::Image::AutoAllocate );

  bool takeSnapshot = false;
  bool halfDepress = false;
  bool fullDepress = false;

  // stream the viewfinder
  sensor.stream ( viewfinder );

  while ( keepGoing )
  {
    // deal with FCam events
    FCam::Event e;
    while ( FCam::getNextEvent ( &e ) )
    {
      cout << e.description << endl;
      switch ( e.type )
      {
      case FCam::Event::FocusPressed:
        if ( autoFocus.idle () )
        {
          autoFocus.startSweep ();
        }
        halfDepress = true;
        break;
      case FCam::Event::FocusReleased:
        halfDepress = false;
        break;
      case FCam::Event::ShutterPressed:
        takeSnapshot = true;
        fullDepress = true;
        break;
      case FCam::Event::ShutterReleased:
        fullDepress = false;
      }
      ;
    }

    // Take a picture once autofocus completes and we have space to store the frame
    if ( takeSnapshot && autoFocus.idle () && writer.savesPending () < 8 )
    {
      // use the metering the viewfinder has been doing
      photo.exposure = viewfinder.exposure;
      photo.gain = viewfinder.gain;
      photo.whiteBalance = viewfinder.whiteBalance;
      sensor.capture ( photo );
      takeSnapshot = false;
    }

    // Drain the queue
    FCam::Frame f;
    do
    {
      f = sensor.getFrame ();

      if ( f.shot ().id == photo.id )
      {
        // Our photo came back, asynchronously save it to disk
        // with a unique filename. We use the exposure start
        // time for now just so we don't have to keep a
        // globally unique numbering.
        if ( !f.image ().valid () )
        {
          printf ( "ERROR: Photo dropped!\n" );
          continue;
        }
        else
        {
          printf ( "Got a frame\n" );
        }

        emit imageCaptured ( f );
      }
      else
      {
        // update the autofocus and metering algorithms
        autoFocus.update ( f );
        autoExpose ( &viewfinder, f );
        autoWhiteBalance ( &viewfinder, f );
        sensor.stream ( viewfinder );
      }
    }
    while ( sensor.framesPending () );
  }
}
