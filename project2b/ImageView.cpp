#include "FeatureSet.h"
#include "ImageView.h"
#include <QPainter>
#include <QMouseEvent>
#include <QPaintEvent>
#include <iostream>

// Create a new ImageView object.
ImageView::ImageView( QWidget * parent ) : QWidget ( parent )
{
  image = NULL;
  features = NULL;

  showFeatures = true;

  mouseDown = false;
}

void ImageView::setBox ( QPoint point1,
                         QPoint point2,
                         QPoint point3,
                         QPoint point4 )
{
  p1 = point1;
  p2 = point2;
  p3 = point3;
  p4 = point4;
}

// Draw the image and features.
void ImageView::paintEvent ( QPaintEvent * )
{
  QPainter painter ( this );

  if ( image != NULL )
  {
    QImage i ( ( uchar * )image->imageData, image->width, image->height,
               image->widthStep, QImage::Format_RGB888 );
    resize ( image->width, image->height );
    painter.drawImage ( QPoint ( 0, 0 ), i );
    painter.setPen ( Qt::red );
    painter.drawLine ( p1, p2 );
    painter.drawLine ( p2, p3 );
    painter.drawLine ( p3, p4 );
    painter.drawLine ( p4, p1 );
  }

  if ( ( features != NULL ) && ( showFeatures ) )
  {
    for ( unsigned int i = 0; i < features->size (); i++ )
    {
      ( *features )[i].draw ( painter );
    }
  }

  // If the user is selecting a box of features, draw the box.
  if ( mouseDown )
  {
    int xMin = xLast;
    int yMin = yLast;
    int width = xCurrent - xLast + 1;
    int height = yCurrent - yLast + 1;

    if ( xCurrent < xLast )
    {
      xMin = xCurrent;
      width = xLast - xCurrent + 1;
    }

    if ( yCurrent < yLast )
    {
      yMin = yCurrent;
      height = yLast - yCurrent + 1;
    }

    painter.setPen ( Qt::white );
    painter.drawRect ( xMin, yMin, width, height );
  }
}

void ImageView::mousePressEvent ( QMouseEvent * event )
{
  switch ( event->button () )
  {
  case Qt::LeftButton:
    if ( features != NULL )
    {
      features->select_point ( event->x (), event->y () );
    }
    repaint ();
    break;
  case Qt::RightButton:
    // Begin to select features by box.
    mouseDown = true;
    xCurrent = xLast = event->x ();
    yCurrent = yLast = event->y ();
    repaint ();
    break;
  default:
    break;
  }
}

void ImageView::mouseReleaseEvent ( QMouseEvent * event )
{
  switch ( event->button () )
  {
  case Qt::RightButton:
    // Select features by box.
    mouseDown = false;

    if ( features != NULL )
    {
      int xMin = xLast;
      int yMin = yLast;
      int xMax = event->x ();
      int yMax = event->y ();

      if ( event->x () < xLast )
      {
        xMin = event->x ();
        xMax = xLast;
      }

      if ( event->y () < yLast )
      {
        yMin = event->y ();
        yMax = yLast;
      }

      features->select_box ( xMin, xMax, yMin, yMax );
    }
    repaint ();
    break;
  default:
    break;
  }
}

void ImageView::mouseMoveEvent ( QMouseEvent * event )
{
  if ( event->buttons () | Qt::RightButton )
  {
    // Update the selection box.
    xCurrent = event->x ();
    yCurrent = event->y ();
    repaint ();
  }
}

// Set the pointer to the image.
void ImageView::setImage ( IplImage * image )
{
  this->image = image;
  p1 = p2 = p3 = p4 = QPoint ( 0, 0 );
  repaint ();
}

// Set the pointer to the feature set.
void ImageView::setFeatures ( FeatureSet * features )
{
  this->features = features;
  repaint ();
}

// Change whether or not the features get drawn.
void ImageView::toggleFeatures ()
{
  showFeatures = ( !showFeatures );
  repaint ();
}
