#ifndef OVERLAY_WIDGET_H
#define OVERLAY_WIDGET_H

#include <QWidget>
#include <QX11Info>
#include "opencv/highgui.h"

#include <FCam/Image.h>
#include <QLabel>

class OverlayWidget : public QWidget {
  public:
    OverlayWidget(QWidget *parent = NULL);
    ~OverlayWidget();

    // If you draw on a widget at the same place as this one, using
    // any color but the one below, it will show through the overlay.
    static QColor colorKey() {return QColor(10, 0, 10);}

    void enable();    
    void disable();
    void setText (const std::string &);
    void displayImage(IplImage *img, void (* render) (IplImage *));
    void startProcess ();
    void endProcess ();
    void startSave ();
    void endSave ();
    
 protected:
    QLabel * label;
    QLabel * saving;
    void resizeEvent(QResizeEvent *);
    void moveEvent(QMoveEvent *);
    void showEvent(QShowEvent *);
    void hideEvent(QHideEvent *);
    bool eventFilter(QObject *receiver, QEvent *event);

    bool filterInstalled;
};

#endif
