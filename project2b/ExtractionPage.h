#ifndef EXTRACTION_PAGE_H
#define EXTRACTION_PAGE_H

#include "ExtractionThread.h"
#include <QWizardPage>
#include <QThread>
#include <QPaintEvent>
#include <opencv/cv.h>
#include <QProgressBar>
#include "Progressable.h"
#include <QLabel>

class FeaturesWizard;

class ExtractionPage : public QWizardPage
{
Q_OBJECT

IplImage * img;
QImage image;
QLabel * canvas;
FeaturesWizard * wizard;
bool complete;
QProgressBar * progress;
ExtractionThread * extractionThread;

void initializePage ();
bool isComplete ();

public:
ExtractionPage ( FeaturesWizard * );

public slots:
void finished ();
};

#endif
