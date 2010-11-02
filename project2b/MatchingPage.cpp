#include "MatchingPage.h"
#include "FeaturesWizard.h"
#include <opencv/cv.h>
#include <QPainter>
#include "MatchingThread.h"
#include <QProgressBar>
#include <QBoxLayout>
#include <QPicture>

MatchingPage::MatchingPage ( FeaturesWizard * parent ) : QWizardPage ( parent ),
  wizard ( parent ), complete ( false )
{
}

void MatchingPage::initializePage ()
{
  if ( !complete )
  {
    image = wizard->getImage ();
    wizard->send_alert ( tr ( "Matching features..." ) );
    matchingThread = new MatchingThread ( this, wizard->getFeatureType (),
                                          wizard->getMatchType (),
                                          wizard->getFeatures () );
    progress = new QProgressBar ( this );
    setLayout ( new QHBoxLayout ( this ) );
    layout ()->addWidget ( progress );
    progress->setMaximum ( 100 );
    QObject::connect ( matchingThread, SIGNAL ( progressUpdate ( int ) ),
                       progress, SLOT ( setValue ( int ) ) );
    QObject::connect ( matchingThread, SIGNAL ( finished () ),
                       this, SLOT ( finished () ) );
    matchingThread->start ();
  }
}

void MatchingPage::finished ()
{
  complete = true;
  progress->hide ();

  const std::vector<FeatureMatch> & matches = matchingThread->getMatches ();
  std::vector<Feature> features = wizard->getFeatures ();
  canvas = new QLabel ( this );
  layout ()->addWidget ( canvas );
  QPicture picture;
  QPainter painter;
  painter.begin ( &picture );                                                   // paint in picture
  painter.drawImage ( QRect ( QPoint ( 0, 0 ), QPoint ( 480, 360 ) ), image );  // draw an ellipse
  for ( size_t i = 0; i < matches.size (); ++i )
  {
    size_t id = matches [i].id1;
    features[id - 1].selected = true;
  }
  for ( size_t i = 0; i < features.size (); ++i )
  {
    features [i].draw ( painter );
  }
  painter.end ();                     // painting done
  canvas->setPicture ( picture );
  canvas->show ();

  wizard->setMatches (
    matchingThread->getMatches () );
  wizard->setMatchIndex ( matchingThread->getMatchIndex () );
  std::stringstream ss;
  ss << "Found " << matchingThread->getName ();
  wizard->send_alert ( tr ( ss.str ().c_str () ) );
}

bool MatchingPage::isComplete ()
{
  return complete;
}
