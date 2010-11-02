#ifndef RESULT_PAGE_H
#define RESULT_PAGE_H

#include <QWizardPage>
#include <QThread>
#include <QPaintEvent>
#include <opencv/cv.h>
#include <QProgressBar>
#include "ResultThread.h"
#include "Progressable.h"
#include <QLabel>
#include <QPoint>

class FeaturesWizard;

class ResultPage : public QWizardPage
{
Q_OBJECT

QImage image;
FeaturesWizard * wizard;
QLabel * canvas;
bool complete;
QProgressBar * progress;
ResultThread * resultThread;

void initializePage ();
bool isComplete ();
QPoint applyHomography ( CvMat *, int, int );

public:
ResultPage ( FeaturesWizard * );

public slots:
void finished ();
};

#endif
