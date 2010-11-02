#include "ExtractionPage.h"
#include "FeaturesWizard.h"
#include <opencv/cv.h>
#include <QPainter>
#include <QProgressBar>
#include <QBoxLayout>
#include <QPicture>

ExtractionPage::ExtractionPage ( FeaturesWizard * parent ) : QWizardPage (
    parent ), wizard ( parent ), complete ( false )
{
}

void ExtractionPage::initializePage ()
{
  if ( !complete )
  {
    img = wizard->getImg ();
    image = wizard->getImage ();
    wizard->send_alert ( tr ( "Extracting features..." ) );
    std::cerr << wizard->getFeatureType () << std::endl;
    extractionThread = new ExtractionThread ( this,
                                              wizard->getFeatureType (), img );
    progress = new QProgressBar ( this );
    setLayout ( new QHBoxLayout ( this ) );
    layout ()->addWidget ( progress );
    progress->setMaximum ( 100 );
    QObject::connect ( extractionThread, SIGNAL ( progressUpdate ( int ) ),
                       progress, SLOT ( setValue ( int ) ) );
    QObject::connect ( extractionThread, SIGNAL ( finished () ),
                       this, SLOT ( finished () ) );
    extractionThread->start ();
  }
}

void ExtractionPage::finished ()
{
  complete = true;
  progress->hide ();
  canvas = new QLabel ( this );
  layout ()->addWidget ( canvas );
  QPicture picture;
  QPainter painter;
  painter.begin ( &picture );                                                   // paint in picture
  painter.drawImage ( QRect ( QPoint ( 0, 0 ), QPoint ( 480, 360 ) ), image );  // draw an ellipse
  const std::vector<Feature> & features = extractionThread->getFeatures ();
  std::cerr << "Features: " << features.size () << std::endl;
  for ( unsigned int i = 0; i < features.size (); i++ )
  {
    features[i].draw ( painter );
  }
  painter.end ();                     // painting done
  canvas->setPicture ( picture );
  canvas->show ();
  wizard->setFeatures ( extractionThread->getFeatures () );
  wizard->send_alert ( tr ( "Features detected" ) );
}

bool ExtractionPage::isComplete ()
{
  return complete;
}
