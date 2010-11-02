#ifndef FEATURES_WIZARD_H
#define FEATURES_WIZARD_H

#include <QWizard>
#include <QWizardPage>
#include <FCam/N900.h>
#include <QMetaType>
#include <opencv/cv.h>
#include <QPaintEvent>
#include "FeaturesMaemo.h"
#include "FeatureSet.h"

class FeaturesWizard : public QWizard
{
Q_OBJECT

FeaturesMaemo * owner;
const FCam::Frame & frame;
IplImage * img;
QImage image;
FeatureType featureType;
MatchType matchType;
FeatureSet features;
std::vector<FeatureMatch> matches;
size_t matchIndex;

public:
FeaturesWizard ( FeaturesMaemo *, const FCam::Frame & );

void done ( int );
const FCam::Frame & getFrame ();
void setImg ( IplImage * );
IplImage * getImg ();
void setImage ( const QImage & );
const QImage & getImage ();
void send_alert ( const QString & );
FeatureType getFeatureType ();
MatchType getMatchType ();
void setFeatures ( const FeatureSet & );
const FeatureSet & getFeatures ();
void setMatchIndex ( size_t );
void setMatches ( const std::vector<FeatureMatch> & );
const std::vector<FeatureMatch> & getMatches ();
size_t getMatchIndex ();

signals:
void alert ( const QString & );
};

#endif
