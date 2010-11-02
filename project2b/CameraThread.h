#ifndef CAMERA_THREAD_H
#define CAMERA_THREAD_H

/** \file */

#include <QThread>
#include <FCam/N900.h>

class InfoWidget;

class CameraThread : public QThread {
Q_OBJECT
CameraThread( QObject * parent = NULL ) : QThread ( parent )
{
  keepGoing = true;
}

public:
void stop ()
{
  printf ( "Stopping!\n" );
  keepGoing = false;
}

protected:
void run ();
private:
bool keepGoing;
FCam::Image overlay;

signals:
void imageCaptured ( FCam::Frame );

public:
static CameraThread & getInstance ();
void setFramebuffer ( FCam::Image );
};

#endif
