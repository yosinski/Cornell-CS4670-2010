#ifndef MATCHING_THREAD_H
#define MATCHING_THREAD_H

#include <QThread>
#include "FeaturesMaemo.h"
#include <QWidget>
#include <opencv/cv.h>
#include "FeatureSet.h"
#include "Progressable.h"

class MatchingThread : public QThread, public Progressable
{
Q_OBJECT

MatchType matchType;
const FeatureSet & features;
FeatureType featureType;
std::vector<FeatureMatch> matches;
size_t index;
std::string name;

void run ();

public:
MatchingThread ( QWidget *, FeatureType, MatchType, const FeatureSet & );
const std::vector<FeatureMatch> & getMatches ();
size_t getMatchIndex ();
const std::string & getName ();

signals:
void progressUpdate ( int );
};
#endif
