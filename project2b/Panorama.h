#ifndef PANORAMA_H
#define PANORAMA_H

#include <QLabel>
#include <QDialog>
#include <QWidget>
#include <opencv/cv.h>
#include <FCam/N900.h>
#include <QProgressBar>
#include <QImage>
#include <QMouseEvent>

class PanoramaThread;
class FeaturesMaemo;

class Panorama : public QDialog
{
Q_OBJECT

IplImage * img1;
IplImage * img2;
FeaturesMaemo * ui;
QProgressBar * progressBar;
PanoramaThread * thread;
bool isready;
bool completed;
QImage image;
QLabel * canvas;

void mousePressEvent ( QMouseEvent * );

public:
Panorama ( FeaturesMaemo * );
~Panorama();
bool ready ();
void emit_alert ( const QString & );

signals:
void alert ( const QString & );
void progressUpdate ( int );

public slots:
void imageCaptured ( const FCam::Frame & );
void computed ();
};

#endif
