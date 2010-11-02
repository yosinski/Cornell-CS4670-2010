#include "FeaturesWizard.h"
#include "FeaturesMaemo.h"
#include "N900Helpers.h"
#include <QVariant>
#include <opencv/cv.h>
#include "CameraThread.h"
#include <QPainter>
#include <iostream>
#include <QBoxLayout>
#include <QProgressBar>
#include "CapturePage.h"
#include "ExtractionPage.h"
#include "FeatureSet.h"
#include "MatchingPage.h"
#include "ResultPage.h"

FeaturesWizard::FeaturesWizard ( FeaturesMaemo *     parent,
                                 const FCam::Frame & frame ) : QWizard (
    parent ), owner ( parent ),
  frame ( frame ), img ( 0 )
{
  resize ( 800, 600 );
  featureType = parent->getFeatureType ();
  matchType = parent->getMatchType ();
  addPage ( new CapturePage ( this ) );
  addPage ( new ExtractionPage ( this ) );
  addPage ( new MatchingPage ( this ) );
  addPage ( new ResultPage ( this ) );
}

void FeaturesWizard::send_alert ( const QString & msg )
{
  emit alert ( msg );
}

void FeaturesWizard::setFeatures ( const FeatureSet & f )
{
  features = f;
}

void FeaturesWizard::setMatchIndex ( size_t i )
{
  matchIndex = i;
}

size_t FeaturesWizard::getMatchIndex ()
{
  return matchIndex;
}

const FeatureSet & FeaturesWizard::getFeatures ()
{
  return features;
}

void FeaturesWizard::done ( int r )
{
  owner->activateViewfinder ();
  QWizard::done ( r );
}

const FCam::Frame & FeaturesWizard::getFrame ()
{
  return frame;
}

void FeaturesWizard::setImg ( IplImage * i )
{
  if ( img )
  {
    cvReleaseImage ( &img );
  }
  img = i;
}

void FeaturesWizard::setImage ( const QImage & i )
{
  image = i;
}

IplImage * FeaturesWizard::getImg ()
{
  return img;
}

const QImage & FeaturesWizard::getImage ()
{
  return image;
}

FeatureType FeaturesWizard::getFeatureType ()
{
  return featureType;
}

MatchType FeaturesWizard::getMatchType ()
{
  return matchType;
}

void FeaturesWizard::setMatches ( const std::vector<FeatureMatch> & m )
{
  matches = m;
}

const std::vector<FeatureMatch> & FeaturesWizard::getMatches ()
{
  return matches;
}
