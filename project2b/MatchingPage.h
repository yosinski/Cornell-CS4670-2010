#ifndef MATCHING_PAGE_H
#define MATCHING_PAGE_H

#include <QWizardPage>
#include <QThread>
#include <QPaintEvent>
#include "MatchingThread.h"
#include <opencv/cv.h>
#include <QProgressBar>
#include "Progressable.h"
#include <QLabel>

class FeaturesWizard;

class MatchingPage : public QWizardPage
{
Q_OBJECT

QImage image;
FeaturesWizard * wizard;
QLabel * canvas;
bool complete;
QProgressBar * progress;
MatchingThread * matchingThread;

void initializePage ();
bool isComplete ();

public:
MatchingPage ( FeaturesWizard * );

public slots:
void finished ();
};

#endif
