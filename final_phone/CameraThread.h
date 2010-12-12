#ifndef CAMERA_THREAD_H
#define CAMERA_THREAD_H

/** \file */

#include <QThread>
#include <opencv/cv.h>
#include <FCam/N900.h>
#include <QMutex>

namespace Plat = FCam::N900;

class OverlayWidget;
class InfoWidget;

class CameraThread : public QThread {
    Q_OBJECT;

    // Make an asynchronous file writer to save images in the background
    FCam::AsyncFileWriter writer;
    Plat::Sensor sensor;
    Plat::Lens lens;
    Plat::Flash flash;

    // The viewfinder shot
    FCam::Shot viewfinder;
    FCam::Shot photo;

    FCam::Image viewfinder_uyvy;
    FCam::Image photo_uyvy;
    FCam::Image viewfinder_raw;
    FCam::Image photo_raw;

    int p;

    void SetGray ();
    void SetYUV ();
    void SetBGR ();

    QMutex mutex;

    IplImage * (* capture)(const FCam::Frame &);
    void (* render) (IplImage *);
    void (* save) (IplImage *, const std::string &);

  public:
    CameraThread(OverlayWidget *o, QObject *parent = NULL) : QThread(parent) {
        overlay = o;
        keepGoing = true;
        p = 0;
    }
        
  public slots:
    void stop() {
        printf("Stopping!\n");
        keepGoing = false;
    }

  protected:
    void run();

  private:
    bool keepGoing;
    OverlayWidget *overlay;
    //Filter process_image;
};

#endif
