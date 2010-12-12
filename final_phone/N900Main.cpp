/** \file */

#include <QApplication>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSlider>
#include <QLabel>

#include "OverlayWidget.h"

#include "CameraWidget.h"

/***********************************************************/
/* Full camera application                                 */
/*                                                         */
/* This example uses QT to create a full camera            */
/* application for the N900. It displays viewfinder frames */
/* using an fbdev overlay.                                 */
/***********************************************************/

int main(int argc, char **argv) {
    QApplication app(argc, argv);

    // Make the main window and a layout for it
    CameraWidget camera;

    // Enter the QT main event loop
    return app.exec();
}
