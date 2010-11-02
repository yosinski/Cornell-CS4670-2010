#ifndef IMAGEVIEW_H
#define IMAGEVIEW_H

#include <opencv/cv.h>
#include <QWidget>
#include <QPaintEvent>
#include <QPoint>

class FeatureSet;

// The ImageView class is responsible for drawing an image and its
// associated features.
class ImageView : public QWidget {
private:
IplImage * image;
FeatureSet * features;
QPoint p1, p2, p3, p4;

bool showFeatures;

bool mouseDown;

int xLast;
int yLast;
int xCurrent;
int yCurrent;

public:
// Create an ImageView object.
ImageView( QWidget * );

// Draw the image and features.
void draw ();
void setBox ( QPoint, QPoint, QPoint, QPoint );
void mouseMoveEvent ( QMouseEvent * );
void mousePressEvent ( QMouseEvent * );
void mouseReleaseEvent ( QMouseEvent * );
void paintEvent ( QPaintEvent * );

// Set the pointer to the image.
void setImage ( IplImage * image );

// Set the pointer to the feature set.
void setFeatures ( FeatureSet * features );

// Change whether or not the features get drawn.
void toggleFeatures ();
};

#endif
