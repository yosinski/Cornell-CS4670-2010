#ifndef CAMERAWIDGET_H
#define CAMERAWIDGET_H

#include <QWidget>
#include <QLabel>
#include "CameraThread.h"

class CameraWidget : public QWidget
{
    Q_OBJECT
    QLabel * label;
    CameraThread * cameraThread;
    OverlayWidget * overlay;
public:
    CameraWidget (QWidget * = 0);
    virtual ~CameraWidget (){}

};

#endif // CAMERAWIDGET_H
