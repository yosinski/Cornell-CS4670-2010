#include "CapturePage.h"
#include <FCam/N900.h>
#include <opencv/cv.h>
#include "FeaturesWizard.h"
#include "N900Helpers.h"
#include <QPainter>
#include <QBoxLayout>
#include <QLabel>
#include <QPicture>
#include <QRect>

CapturePage::CapturePage ( FeaturesWizard * parent ) : QWizardPage ( parent ),
  wizard ( parent )
{
  const FCam::Frame & frame = wizard->getFrame ();
  IplImage * img = capture_rgb24 ( frame );

  wizard->setImg ( img );
}

void CapturePage::initializePage ()
{
  IplImage * img = wizard->getImg ();

  image = QImage ( ( unsigned char * )img->imageData,
                   img->width,
                   img->height,
                   img->widthStep,
                   QImage::Format_RGB888 );
  wizard->setImage ( image );
  wizard->send_alert ( tr ( "Image captured" ) );
  QHBoxLayout * layout = new QHBoxLayout ( this );
  canvas = new QLabel ( this );
  setLayout ( layout );
  layout->addWidget ( canvas );
  QPicture picture;
  QPainter painter;
  painter.begin ( &picture );                                                   // paint in picture
  painter.drawImage ( QRect ( QPoint ( 0, 0 ), QPoint ( 480, 360 ) ), image );  // draw an ellipse
  painter.end ();                                                               // painting done
  canvas->setPicture ( picture );
  canvas->show ();
}
