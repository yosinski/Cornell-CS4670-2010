#include "ResultPage.h"
#include "FeaturesWizard.h"
#include <opencv/cv.h>
#include <QPainter>
#include "ResultThread.h"
#include <QProgressBar>
#include <QBoxLayout>
#include <QPicture>
#include "ImageDatabase.h"
#include <opencv/highgui.h>

ResultPage::ResultPage ( FeaturesWizard * parent ) : QWizardPage ( parent ),
  wizard ( parent ), complete ( false )
{
}

void ResultPage::initializePage ()
{
  if ( !complete )
  {
    image = wizard->getImage ();
    wizard->send_alert ( tr ( "Computing result..." ) );
    resultThread = new ResultThread ( this,
                                      wizard->getFeatureType (),
                                      wizard->getMatches (),
                                      wizard->getFeatures (),
                                      wizard->getMatchIndex () );
    progress = new QProgressBar ( this );
    setLayout ( new QHBoxLayout ( this ) );
    layout ()->addWidget ( progress );
    progress->setMaximum ( 100 );
    QObject::connect ( resultThread, SIGNAL ( progressUpdate ( int ) ),
                       progress, SLOT ( setValue ( int ) ) );
    QObject::connect ( resultThread, SIGNAL ( finished () ),
                       this, SLOT ( finished () ) );
    resultThread->start ();
  }
}

QPoint ResultPage::applyHomography ( CvMat * h, int x, int y )
{
  std::cerr << x << " " << y << std::endl;
  for ( int i = 0; i < h->height; ++i )
  {
    for ( int j = 0; j < h->width; ++j )
    {
      std::cerr << CV_MAT_ELEM ( *h, float, i, j ) << " ";
    }
    std::cerr << std::endl;
  }
  float xPrime = CV_MAT_ELEM ( *h, float, 0, 0 ) * x + CV_MAT_ELEM ( *h,
                                                                     float,
                                                                     0,
                                                                     1 ) * y +
                 CV_MAT_ELEM ( *h, float, 0, 2 );
  float yPrime = CV_MAT_ELEM ( *h, float, 1, 0 ) * x + CV_MAT_ELEM ( *h,
                                                                     float,
                                                                     1,
                                                                     1 ) * y +
                 CV_MAT_ELEM ( *h, float, 1, 2 );
  float w = CV_MAT_ELEM ( *h, float, 2, 0 ) * x + CV_MAT_ELEM ( *h,
                                                                float,
                                                                2,
                                                                1 ) * y +
            CV_MAT_ELEM ( *h, float, 2, 2 );
  std::cerr << xPrime << " " << yPrime << " " << w << std::endl;
  QPoint prime ( xPrime / w, yPrime / w );
  return prime;
}

void ResultPage::finished ()
{
  complete = true;
  progress->hide ();

  CvMat * h = resultThread->getHomography ();
  ImageDatabase db;
  std::stringstream ss;
  ss << wizard->getFeatureType () << ".db";
  db.load ( ss.str ().c_str (), false );
  std::string name = db[wizard->getMatchIndex ()].name;
  std::cerr << name.c_str () << std::endl;
  IplImage * i = cvLoadImage ( name.c_str () );
  int width = i->width;
  int height = i->height;
  cvReleaseImage ( &i );
  QPoint point1 = applyHomography ( h, 0, 0 );
  QPoint point2 = applyHomography ( h, width, 0 );
  QPoint point3 = applyHomography ( h, width, height );
  QPoint point4 = applyHomography ( h, 0, height );
  canvas = new QLabel ( this );
  layout ()->addWidget ( canvas );
  QPicture picture;
  QPainter painter;
  painter.begin ( &picture );                                                   // paint in picture
  painter.drawImage ( QPoint ( 0, 0 ), image );
  painter.setPen ( Qt::red );
  painter.drawLine ( point1, point2 );
  painter.drawLine ( point2, point3 );
  painter.drawLine ( point3, point4 );
  painter.drawLine ( point4, point1 );
  painter.end ();                                                               // painting done
  canvas->setPicture ( picture );
  canvas->show ();

  wizard->send_alert ( tr ( "Result complete" ) );
}

bool ResultPage::isComplete ()
{
  return complete;
}
