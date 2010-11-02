#include "Panorama.h"
#include "N900Helpers.h"
#include <QBoxLayout>
#include "PanoramaThread.h"
#include "FeaturesMaemo.h"
#include <QPainter>
#include <QPicture>
#include <opencv/highgui.h>

Panorama::Panorama ( FeaturesMaemo * parent )
  : QDialog ( parent ), img1 ( 0 ), img2 ( 0 ), ui ( parent ), isready ( false ),
  completed ( false )
{
  progressBar = new QProgressBar ( this );
  progressBar->setMaximum ( 100 );
  QHBoxLayout * layout = new QHBoxLayout ( this );
  layout->addWidget ( progressBar );
  setLayout ( layout );
}

Panorama::~Panorama()
{
  if ( img1 )
  {
    cvReleaseImage ( &img1 );
  }
  if ( img2 )
  {
    cvReleaseImage ( &img2 );
  }
}

void Panorama::emit_alert ( const QString & msg )
{
  emit alert ( msg );
}

bool Panorama::ready ()
{
  return isready;
}

void Panorama::mousePressEvent ( QMouseEvent * event )
{
  if ( completed )
  {
    ui->activateViewfinder ();
  }
  QDialog::mousePressEvent ( event );
}

void Panorama::imageCaptured ( const FCam::Frame & frame )
{
  IplImage * img = capture_rgb24 ( frame );

  if ( !img1 )
  {
    img1 = img;
  }
  else
  {
    img2 = img;
    isready = true;
    thread = new PanoramaThread ( this, img1, img2,
                                  ui->getFeatureType (), ui->getMatchType () );
    QObject::connect ( thread, SIGNAL ( progressUpdate ( int ) ),
                       progressBar, SLOT ( setValue ( int ) ) );
    QObject::connect ( thread, SIGNAL ( finished () ),
                       this, SLOT ( computed () ) );
    thread->start ();
  }
}

void Panorama::computed ()
{
  completed = true;
  IplImage * output = thread->getComposite ();
  IplImage * temp = cvCreateImage ( cvGetSize ( output ), IPL_DEPTH_8U, 3 );
  cvConvertScale ( output, temp, 255, 0 );
  cvReleaseImage ( &output );
  output = temp;
  image = QImage ( ( const uchar * )output->imageData,
                   output->width,
                   output->height,
                   output->widthStep,
                   QImage::Format_RGB888 );
  canvas = new QLabel ( this );
  progressBar->hide ();
  layout ()->addWidget ( canvas );
  QPicture picture;
  QPainter painter;
  painter.begin ( &picture );
  painter.drawImage ( QRect ( QPoint ( 0, 0 ), QPoint ( 800, 600 ) ), image );
  painter.end ();
  canvas->setPicture ( picture );
  canvas->show ();
  cvCvtColor ( output, output, CV_RGB2BGR );
  std::stringstream ss;
  ss << "/home/user/MyDocs/ " << rand () << ".png";
  cvSaveImage ( ss.str ().c_str (), output );
}
