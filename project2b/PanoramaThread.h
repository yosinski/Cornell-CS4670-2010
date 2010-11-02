#ifndef PANORAMA_THREAD_H
#define PANORAMA_THREAD_H

#include <QThread>
#include <QWidget>
#include "FeaturesMaemo.h"
#include <opencv/cv.h>
#include "Progressable.h"

class Panorama;

class PanoramaThread : public QThread, public Progressable
{
Q_OBJECT

Panorama * panorama;
IplImage * img1;
IplImage * img2;
IplImage * output;
FeatureType featureType;
MatchType matchType;

protected:
void run ();

public:
PanoramaThread ( Panorama *, IplImage *, IplImage *, FeatureType, MatchType );
IplImage * getComposite ();

signals:
void progressUpdate ( int );
};

#endif
