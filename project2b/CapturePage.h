#ifndef CAPTURE_PAGE_H
#define CAPTURE_PAGE_H

#include <QWizardPage>
#include <QImage>
#include <QPaintEvent>
#include <QPainter>
#include <QLabel>

class FeaturesWizard;

class CapturePage : public QWizardPage
{
FeaturesWizard * wizard;
QImage image;
QLabel * canvas;

void initializePage ();

public:
CapturePage ( FeaturesWizard * );
};

#endif
