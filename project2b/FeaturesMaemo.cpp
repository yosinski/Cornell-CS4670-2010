#include "FeaturesMaemo.h"
#include "FeaturesWizard.h"
#include "Viewfinder.h"
#include <QFileDialog>
#include <QWidget>
#include <QMessageBox>
#include <QStatusBar>
#include <QHBoxLayout>
#include <QComboBox>
#include <QVBoxLayout>
#include <QPushButton>
#include <QDialog>
#include <sstream>
#include <iostream>
#include "N900Helpers.h"
#include "Panorama.h"

// Create and initialize the Maemo.
FeaturesMaemo::FeaturesMaemo ( QWidget * parent ) : QMainWindow ( parent ),
  featureType ( SURF ), matchType ( Ratio ), panorama ( false )
{
  QStatusBar * statusBar = new QStatusBar ( this );

  QObject::connect ( this, SIGNAL ( alert ( const QString & ) ),
                     statusBar, SLOT ( showMessage ( const QString & ) ) );
  setStatusBar ( statusBar );

  // Create the main window.
  setWindowTitle ( tr ( "Detector" ) );

  stack = new QStackedWidget ( this );
  setCentralWidget ( stack );

  // Create the menu bar.
  QMenuBar * menuBar = new QMenuBar ( this );

  QAction * setFeatureType = new QAction ( tr ( "Set Feature Type" ), menuBar );
  QObject::connect ( setFeatureType, SIGNAL ( triggered () ),
                     this, SLOT ( setFeatureTypeAction () ) );
  menuBar->addAction ( setFeatureType );

  QAction * setMatchType = new QAction ( tr ( "Set Match Type" ), menuBar );
  QObject::connect ( setMatchType, SIGNAL ( triggered () ),
                     this, SLOT ( setMatchTypeAction () ) );
  menuBar->addAction ( setMatchType );

  QAction * setMode = new QAction ( tr ( "Set Mode" ), menuBar );
  QObject::connect ( setMode, SIGNAL ( triggered () ),
                     this, SLOT ( setModeAction () ) );
  menuBar->addAction ( setMode );

  setMenuBar ( menuBar );

  viewFinder = new Viewfinder ( this );
  stack->addWidget ( viewFinder );

  featureTypes ["Dummy"] = Dummy;
  featureTypes ["MOPS"] = MOPS;
  featureTypes ["SURF"] = SURF;

  matchTypes ["SSD"] = SSD;
  matchTypes ["Ratio"] = Ratio;

  QObject::connect ( viewFinder, SIGNAL ( alert ( const QString & ) ),
                     statusBar, SLOT ( showMessage ( const QString & ) ) );
  activateViewfinder ();
}

void FeaturesMaemo::activateViewfinder ()
{
  stack->setCurrentIndex ( 0 );
  qRegisterMetaType<FCam::Frame>( "FCam::Frame" );
  QObject::connect ( &CameraThread::getInstance (),
                     SIGNAL ( imageCaptured ( const FCam::Frame & ) ), this,
                     SLOT ( pictureTaken ( const FCam::Frame & ) ) );
  viewFinder->show ();
}

void FeaturesMaemo::deactivateViewfinder ()
{
  QObject::disconnect ( &CameraThread::getInstance (),
                        SIGNAL ( imageCaptured ( const FCam::Frame & ) ), this,
                        SLOT ( pictureTaken ( const FCam::Frame & ) ) );
  stack->setCurrentIndex ( 1 );
}

FeatureType FeaturesMaemo::getFeatureType ()
{
  return featureType;
}

MatchType FeaturesMaemo::getMatchType ()
{
  return matchType;
}

void FeaturesMaemo::setFeatureTypeAction ()
{
  QDialog * dialog = new QDialog ( this );

  dialog->setModal ( true );
  dialog->setWindowTitle ( tr ( "Feature Type" ) );
  QVBoxLayout * layout = new QVBoxLayout ( dialog );
  QPushButton * okay = new QPushButton ( tr ( "Okay" ), dialog );

  QComboBox * combobox = new QComboBox ( dialog );
  for ( std::map<QString, FeatureType>::iterator i = featureTypes.begin ();
        i != featureTypes.end (); ++i )
  {
    combobox->addItem ( i->first );
    if ( featureType == i->second )
    {
      combobox->setCurrentIndex ( combobox->count () - 1 );
    }
  }
  QObject::connect ( combobox, SIGNAL ( currentIndexChanged ( const QString & ) ),
                     this, SLOT ( updateFeatureType ( const QString & ) ) );
  layout->addWidget ( combobox );
  layout->addWidget ( okay );
  QObject::connect ( okay, SIGNAL ( clicked () ),
                     dialog, SLOT ( close () ) );
  dialog->setLayout ( layout );
  dialog->show ();
}

void FeaturesMaemo::setModeAction ()
{
  QDialog * dialog = new QDialog ( this );

  dialog->setModal ( true );
  dialog->setWindowTitle ( tr ( "Mode" ) );
  QVBoxLayout * layout = new QVBoxLayout ( dialog );
  QPushButton * okay = new QPushButton ( tr ( "Okay" ), dialog );

  QComboBox * combobox = new QComboBox ( dialog );
  combobox->addItem ( "Object Detection" );
  combobox->addItem ( "Panorama" );
  combobox->setEditable ( false );
  combobox->setCurrentIndex ( panorama ? 1 : 0 );
  QObject::connect ( combobox, SIGNAL ( currentIndexChanged ( const QString & ) ),
                     this, SLOT ( updateMode ( const QString & ) ) );
  layout->addWidget ( combobox );
  layout->addWidget ( okay );
  QObject::connect ( okay, SIGNAL ( clicked () ),
                     dialog, SLOT ( close () ) );
  dialog->setLayout ( layout );
  dialog->show ();
}


void FeaturesMaemo::updateFeatureType ( const QString & name )
{
  featureType = featureTypes [name];
  std::stringstream ss;
  ss << "Feature type set to " << name.toStdString ();
  emit alert ( tr ( ss.str ().c_str () ) );
}

void FeaturesMaemo::updateMode ( const QString & name )
{
  panorama = ( name == "Panorama" );
  std::stringstream ss;
  ss << "Mode set to " << name.toStdString ();
  emit alert ( tr ( ss.str ().c_str () ) );
}

void FeaturesMaemo::setMatchTypeAction ()
{
  QDialog * dialog = new QDialog ( this );

  dialog->setModal ( true );
  dialog->setWindowTitle ( tr ( "Match Type" ) );
  QVBoxLayout * layout = new QVBoxLayout ( dialog );
  QPushButton * okay = new QPushButton ( tr ( "Okay" ), dialog );

  QComboBox * combobox = new QComboBox ( dialog );
  for ( std::map<QString, MatchType>::iterator i = matchTypes.begin ();
        i != matchTypes.end (); ++i )
  {
    combobox->addItem ( i->first );
    if ( matchType == i->second )
    {
      combobox->setCurrentIndex ( combobox->count () - 1 );
    }
  }
  combobox->setEditable ( false );
  QObject::connect ( combobox, SIGNAL ( currentIndexChanged ( const QString & ) ),
                     this, SLOT ( updateMatchType ( const QString & ) ) );
  layout->addWidget ( combobox );
  layout->addWidget ( okay );
  QObject::connect ( okay, SIGNAL ( clicked () ),
                     dialog, SLOT ( close () ) );
  dialog->setLayout ( layout );
  dialog->show ();
}

void FeaturesMaemo::updateMatchType ( const QString & name )
{
  matchType = matchTypes [name];
  std::stringstream ss;
  ss << "Match type set to " << name.toStdString ();
  emit alert ( tr ( ss.str ().c_str () ) );
}

void FeaturesMaemo::pictureTaken ( const FCam::Frame & frame )
{
  if ( stack->count () == 2 )
  {
    QWidget * widget = stack->widget ( 1 );
    stack->removeWidget ( widget );
  }
  if ( !panorama )
  {
    QWidget * wizard = new FeaturesWizard ( this, frame );
    stack->addWidget ( wizard );
    QObject::connect ( wizard, SIGNAL ( alert ( const QString & ) ),
                       statusBar (), SLOT ( showMessage ( const QString & ) ) );
    wizard->show ();
    deactivateViewfinder ();
  }
  else
  {
    if ( !panorama_widget || panorama_widget->ready () )
    {
      panorama_widget = new Panorama ( this );
    }
    panorama_widget->imageCaptured ( frame );
    if ( panorama_widget->ready () )
    {
      stack->addWidget ( panorama_widget );
      QObject::connect ( panorama_widget, SIGNAL ( alert ( const QString & ) ),
                         statusBar (), SLOT ( showMessage ( const QString & ) ) );
      panorama_widget->show ();
      deactivateViewfinder ();
    }
  }
}
