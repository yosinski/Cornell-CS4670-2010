#ifndef RESULT_THREAD_H
#define RESULT_THREAD_H

#include <QThread>
#include "FeaturesMaemo.h"
#include <QWidget>
#include <opencv/cv.h>
#include "FeatureSet.h"
#include "Progressable.h"

class ResultThread : public QThread, public Progressable
{
Q_OBJECT

const std::vector<FeatureMatch> & matches;
const std::vector<Feature> & f1;
size_t matchIndex;
FeatureType featureType;
CvMat * h;

void run ();

public:
ResultThread ( QWidget *, FeatureType, const std::vector<
                 FeatureMatch> &, const std::vector<
                 Feature> &, size_t );
CvMat * getHomography ();

signals:
void progressUpdate ( int );
};
#endif
