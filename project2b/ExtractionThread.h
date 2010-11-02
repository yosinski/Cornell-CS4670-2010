#ifndef EXTRACTION_THREAD_H
#define EXTRACTION_THREAD_H

#include <QThread>
#include "FeaturesMaemo.h"
#include <QWidget>
#include <opencv/cv.h>
#include "FeatureSet.h"
#include "Progressable.h"

class ExtractionThread : public QThread, public Progressable
{
Q_OBJECT

FeatureSet features;
FeatureType featureType;
IplImage * img;

void run ();

public:
ExtractionThread ( QWidget *, FeatureType, IplImage * );
const FeatureSet & getFeatures ();

signals:
void progressUpdate ( int );
};


#endif
