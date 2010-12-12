#include "OverlayWidget.h"
#include <QEvent>
#include <stdlib.h>
#include <stdio.h>
#include "Framebuffer.h"

OverlayWidget::OverlayWidget(QWidget *par) : QWidget(par)
{
  /* Make QT do the work of keeping the overlay the magic color  */
  QWidget::setBackgroundRole(QPalette::Window);
  QWidget::setAutoFillBackground(true);
  QPalette overlayPalette = QWidget::palette();
  overlayPalette.setColor
    (QPalette::Window,
    colorKey());
  QWidget::setPalette(overlayPalette);

  Framebuffer::GetInstance().SetColorKey (colorKey());

  // HACK HACK HACK
  // For some reason, if these labels use the overlay as their parent,
  // only the first character is rendered.  I'm making this overlay's
  // parent their parent.  This is going to mess up their location, so
  // I'm going to manually insert it.
  label = new QLabel (par);
  saving = new QLabel (par);
  saving->move (395, 240);
  label->move (75, 0);

  filterInstalled = false;
}


bool OverlayWidget::eventFilter(QObject *, QEvent *event)
{
  if (event->type() == QEvent::Move ||
    event->type() == QEvent::Resize ||
    event->type() == QEvent::Show)
  {
    enable();
  }
  else if (event->type() == QEvent::Hide)
  {
    disable();
  }

  // We don't capture this event, it should be propagated as normal
  return false;
}


void OverlayWidget::showEvent(QShowEvent *)
{
  enable();
}


void OverlayWidget::hideEvent(QHideEvent *)
{
  disable();
}


void OverlayWidget::resizeEvent(QResizeEvent *)
{
  enable();
}


void OverlayWidget::moveEvent(QMoveEvent *)
{
  enable();
}


OverlayWidget::~OverlayWidget()
{
  Framebuffer::GetInstance().Disable();
}


void OverlayWidget::enable()
{
  Framebuffer::GetInstance().Enable (this);

  if (!filterInstalled)
  {
    // if anything moves above me, we need to know about it to update the overlay
    for (QObject *obj = parent(); obj; obj = obj->parent())
    {
      obj->installEventFilter(this);
    }
    filterInstalled = true;
  }
}

void OverlayWidget::displayImage(IplImage *img, void (* render) (IplImage *))
{
  printf("Press enter to continue.\n");
  render(img);
  getchar();
}

void OverlayWidget::disable()
{
  Framebuffer::GetInstance().Disable();
}

void OverlayWidget::setText (const std::string & text)
{
  label->setText (QString::fromStdString (text));
}

void OverlayWidget::startProcess ()
{
  saving->setText ("Working...");
}

void OverlayWidget::endProcess ()
{
  saving->setText ("");
}

void OverlayWidget::startSave ()
{
  saving->setText ("Saving...");
}

void OverlayWidget::endSave ()
{
  saving->setText ("");
}
