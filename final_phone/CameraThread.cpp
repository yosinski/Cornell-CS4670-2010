/** \file */

#include <qmetatype.h>
#include "CameraThread.h"
 
#include <FCam/N900.h>

#include <vector>
#include <iostream>

#include "OverlayWidget.h"

#include "opencv/highgui.h"

#include "Framebuffer.h"
#include "N900Helpers.h"

using namespace std;

namespace Plat = FCam::N900;

void CameraThread::run() {
    
    // tell the sensor that the flash and the lens will be tagging
    // frames that come back from it
    sensor.attach(&flash);
    sensor.attach(&lens);
    
    // Make a helper autofocus object
    FCam::AutoFocus autoFocus(&lens);

    // Make an action to fire the flash
    Plat::Flash::FireAction fire(&flash);
    fire.duration = flash.minDuration();          // flash briefly
    fire.time = photo.exposure - fire.duration; // at the end of the exposure
    fire.brightness = flash.maxBrightness();      // at full power

    viewfinder_uyvy = FCam::Image(640, 480, FCam::UYVY, (unsigned char *)Framebuffer::GetInstance().GetBuffer());//FCam::Image::AutoAllocate);
    photo_uyvy = FCam::Image(640, 480, FCam::UYVY, FCam::Image::AutoAllocate);
    viewfinder_raw = FCam::Image(640, 480, FCam::RAW, FCam::Image::AutoAllocate);
    photo_raw = FCam::Image(648, 492, FCam::RAW, FCam::Image::AutoAllocate);

    viewfinder.exposure = 40000;
    viewfinder.gain = 1.0f;
    // run at 25 fps
    viewfinder.frameTime = 40000;
    // dump image data directly into the frame buffer
    // enable histograms and sharpness maps
    viewfinder.histogram.enabled = true;
    viewfinder.histogram.region = FCam::Rect(0, 0, 640, 480);
    viewfinder.sharpness.enabled = true;
    viewfinder.sharpness.size = FCam::Size(16, 12);

    //set_filter (Filters::GetInstance().GetFilter(1));
    mutex.lock();
    while (sensor.framesPending())
    {
      sensor.getFrame();
    }
    overlay->disable();
    SetBGR();
    overlay->enable();
    mutex.unlock();

    bool takeSnapshot = false;
    bool halfDepress = false;
    bool fullDepress = false;

    sensor.stream (viewfinder);
    
    while (keepGoing) {
        // deal with FCam events
        FCam::Event e;
        while (FCam::getNextEvent(&e)) {
            cout << e.description << endl;
            switch(e.type) {
            case FCam::Event::FocusPressed:
                if (autoFocus.idle()) autoFocus.startSweep();       
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
            };
        }
    
        // Take a picture once autofocus completes and we have space to store the frame
        if (takeSnapshot && autoFocus.idle() && writer.savesPending() < 8) {
            // use the metering the viewfinder has been doing
            photo.exposure  = viewfinder.exposure;
            photo.gain      = viewfinder.gain;
            photo.whiteBalance = viewfinder.whiteBalance;
            sensor.capture(photo);
            takeSnapshot = false;
        }
    
        // Drain the queue
        FCam::Frame f;
        mutex.lock();
        do {
            f = sensor.getFrame();
        
            if (f.shot().id == photo.id) {
                // Our photo came back, asynchronously save it to disk
                // with a unique filename. We use the exposure start
                // time for now just so we don't have to keep a
                // globally unique numbering.
                if (!f.image().valid()) {
                    printf("ERROR: Photo dropped!\n");
                    continue;
                } else {
                    printf("Got a frame\n");
                }
	              // Save it as a DNG
	              char fname[256];

	              // Save it as a JPEG too
	              snprintf(fname, 255, "/home/user/MyDocs/photo_%s.jpg", 
	                        f.exposureStartTime().toString().c_str());
                IplImage * img = capture (f);
                
                overlay->startProcess ();
                process_image (img);
                overlay->endProcess ();
                
                sensor.stopStreaming();
                overlay->displayImage(img, render );
                sensor.start();
                
                overlay->startSave ();
                save (img, fname);
                overlay->endSave ();
		            cvReleaseImage (&img);
            } 
            else
            {
                //IplImage * img = capture (f);
                //render (img);
	              //cvReleaseImage (&img);

                // update the autofocus and metering algorithms     
                autoFocus.update(f);
                autoExpose(&viewfinder, f);
                autoWhiteBalance(&viewfinder, f);
                sensor.stream (viewfinder);
            }
        } while (sensor.framesPending());
        mutex.unlock();
    }
}

void CameraThread::SetYUV()
{
    viewfinder.image = viewfinder_uyvy;
    photo.image = photo_uyvy;
    capture = capture_yuv24;
    render = render_yuv24;
    save = save_yuv24;
    Framebuffer::GetInstance().SetYUV();
}

void CameraThread::SetGray()
{
    viewfinder.image = viewfinder_uyvy;
    photo.image = photo_uyvy;
    capture = capture_gray;
    render = render_gray;
    save = save_gray;
    Framebuffer::GetInstance().SetGray();
}

void CameraThread::SetBGR()
{
    viewfinder.image = viewfinder_uyvy;//viewfinder_raw;
    photo.image = photo_uyvy;
    capture = capture_bgr24;
    render = render_bgr24;
    save = save_bgr24;
    Framebuffer::GetInstance().SetBGR();
}

void CameraThread::process_image(IplImage* img)
{
  sleep(1);
}
