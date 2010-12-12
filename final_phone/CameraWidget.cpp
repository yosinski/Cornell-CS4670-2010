#include "CameraWidget.h"
#include <QLayout>
#include <QSlider>
#include "OverlayWidget.h"
#include <QPushButton>
#include "CameraThread.h"

CameraWidget::CameraWidget (QWidget * parent):QWidget (parent)
{
    label = new QLabel (this);
    QHBoxLayout *layout = new QHBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);

    // Make an overlay for displaying viewfinder frames
    overlay = new OverlayWidget(this);
    overlay->setFixedSize(640, 480);
    layout->addWidget(overlay);

    // Make some buttons down the right
    //QVBoxLayout *buttonLayout = new QVBoxLayout;
    //layout->addLayout(buttonLayout);
    //QPushButton *quitButton = new QPushButton("X");
    //quitButton->setFixedSize(80, 64);
    //buttonLayout->addWidget(quitButton);
    //buttonLayout->addStretch(1);

    setLayout(layout);

    // Show the app full screen
    showFullScreen();

    // Make a thread that controls the camera
    cameraThread = new CameraThread(overlay);

    // Hook up the quit button to stop the camera thread
    //QObject::connect(quitButton, SIGNAL(clicked()),
    //                 cameraThread, SLOT(stop()));
                     
    // Once the camera thread stops, quit the app
    QObject::connect(cameraThread, SIGNAL(finished()),
                     this, SLOT(close()));

    // Launch the camera thread
    cameraThread->start();

    //start_filter1();
}


