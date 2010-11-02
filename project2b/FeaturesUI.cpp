#include "FeaturesUI.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QStatusBar>
#include <QHBoxLayout>
#include <iostream>

const int minWidth = 600;
const int minHeight = 300;

// Create and initialize the UI.
FeaturesUI::FeaturesUI ( QWidget * parent ) : QMainWindow ( parent )
{
  doc = new FeaturesDoc ( this );
  QStatusBar * statusBar = new QStatusBar ( this );
  setStatusBar ( statusBar );
  QObject::connect ( doc, SIGNAL ( alert ( QString ) ),
                     statusBar, SLOT ( showMessage ( const QString & ) ) );
  // Create the main window.
  setFixedSize ( minWidth, minHeight );
  setWindowTitle ( "Features" );

  // Create the menu bar.
  QMenuBar * menuBar = new QMenuBar ( this );

  menuBar->addMenu ( createFileMenu ( menuBar ) );
  menuBar->addMenu ( createImageMenu ( menuBar ) );
  menuBar->addMenu ( createOptionsMenu ( menuBar ) );
  menuBar->addMenu ( createHelpMenu ( menuBar ) );

  setMenuBar ( menuBar );

  QWidget * widget = new QWidget ( this );
  QHBoxLayout * layout = new QHBoxLayout ( widget );

  // Create the subwindows for viewing the query and result images.
  queryView = new ImageView ( this );
  layout->addWidget ( queryView );

  resultView = new ImageView ( this );
  layout->addWidget ( resultView );

  widget->setLayout ( layout );

  setCentralWidget ( widget );
}

QMenu * FeaturesUI::createFileMenu ( QWidget * parent )
{
  QMenu * file = new QMenu ( "&File", parent );
  QAction * loadQueryImage = new QAction ( "&Load Query Image", file );

  file->addAction ( loadQueryImage );
  QObject::connect ( loadQueryImage, SIGNAL ( triggered () ),
                     this, SLOT ( cb_load_query_image () ) );
  file->addMenu ( createLoadQueryFeaturesMenu ( file ) );
  file->addMenu ( createLoadImageDatabaseMenu ( file ) );
  QAction * exit = new QAction ( "&Exit", file );
  file->addAction ( exit );
  QObject::connect ( exit, SIGNAL ( triggered () ),
                     this, SLOT ( cb_exit () ) );
  return file;
}

QMenu * FeaturesUI::createLoadQueryFeaturesMenu ( QWidget * parent )
{
  QMenu * loadQueryFeatures = new QMenu ( "&Load Query Features", parent );
  QAction * normal = new QAction ( "&Normal", loadQueryFeatures );

  loadQueryFeatures->addAction ( normal );
  QObject::connect ( normal, SIGNAL ( triggered () ),
                     this, SLOT ( cb_load_query_features () ) );
  QAction * sift = new QAction ( "&SIFT", loadQueryFeatures );
  loadQueryFeatures->addAction ( sift );
  QObject::connect ( sift, SIGNAL ( triggered () ),
                     this, SLOT ( cb_load_query_features_sift () ) );
  return loadQueryFeatures;
}

QMenu * FeaturesUI::createLoadImageDatabaseMenu ( QWidget * parent )
{
  QMenu * loadImageDatabase = new QMenu ( "&Load Image Database", parent );
  QAction * normal = new QAction ( "&Normal", loadImageDatabase );

  loadImageDatabase->addAction ( normal );
  QObject::connect ( normal, SIGNAL ( triggered () ),
                     this, SLOT ( cb_load_image_database () ) );
  QAction * sift = new QAction ( "&SIFT", loadImageDatabase );
  loadImageDatabase->addAction ( sift );
  QObject::connect ( sift, SIGNAL ( triggered () ),
                     this, SLOT ( cb_load_image_database_sift () ) );
  return loadImageDatabase;
}

QMenu * FeaturesUI::createImageMenu ( QWidget * parent )
{
  QMenu * image = new QMenu ( "&Image", parent );
  QAction * selectAllFeatures = new QAction ( "&Select All Features", image );

  image->addAction ( selectAllFeatures );
  QObject::connect ( selectAllFeatures, SIGNAL ( triggered () ),
                     this, SLOT ( cb_select_all_features () ) );
  QAction * deselectAllFeatures = new QAction ( "&Deselect All Features",
                                                image );
  image->addAction ( deselectAllFeatures );
  QObject::connect ( deselectAllFeatures, SIGNAL ( triggered () ),
                     this, SLOT ( cb_deselect_all_features () ) );
  QAction * performQuery = new QAction ( "&Perform Query", image );
  image->addAction ( performQuery );
  QObject::connect ( performQuery, SIGNAL ( triggered () ),
                     this, SLOT ( cb_perform_query () ) );
  return image;
}

QMenu * FeaturesUI::createOptionsMenu ( QWidget * parent )
{
  QMenu * options = new QMenu ( "&Options", parent );

  options->addMenu ( createSelectMatchAlgorithmMenu ( options ) );
  QAction * toggleFeatures = new QAction ( "&Toggle Features", options );
  options->addAction ( toggleFeatures );
  QObject::connect ( toggleFeatures, SIGNAL ( triggered () ),
                     this, SLOT ( cb_toggle_features () ) );
  return options;
}

QMenu * FeaturesUI::createSelectMatchAlgorithmMenu ( QWidget * parent )
{
  QMenu * selectMatchAlgorithm = new QMenu ( "&Select Match Algorithm", parent );
  QAction * algorithm1 = new QAction ( "&Algorithm 1", selectMatchAlgorithm );

  selectMatchAlgorithm->addAction ( algorithm1 );
  QObject::connect ( algorithm1, SIGNAL ( triggered () ),
                     this, SLOT ( cb_match_algorithm_1 () ) );
  QAction * algorithm2 = new QAction ( "&Algorithm 2", selectMatchAlgorithm );
  selectMatchAlgorithm->addAction ( algorithm2 );
  QObject::connect ( algorithm2, SIGNAL ( triggered () ),
                     this, SLOT ( cb_match_algorithm_2 () ) );
  return selectMatchAlgorithm;
}

QMenu * FeaturesUI::createHelpMenu ( QWidget * parent )
{
  QMenu * help = new QMenu ( "&Help", parent );
  QAction * about = new QAction ( "&About", help );

  help->addAction ( about );
  QObject::connect ( about, SIGNAL ( triggered () ),
                     this, SLOT ( cb_about () ) );
  return help;
}

void FeaturesUI::set_box ( QPoint p1, QPoint p2, QPoint p3, QPoint p4 )
{
  queryView->setBox ( p1, p2, p3, p4 );
}

// Resize the query and result image windows.
void FeaturesUI::resize_windows ( int w1, int w2, int h )
{
  setFixedSize ( MAX ( w1 + w2 + 25, minWidth ), MAX ( h + 25, minHeight ) );
  queryView->setFixedSize ( w1, h );
  resultView->setFixedSize ( w2, h );
}

// Set the query and result image pointers.
void FeaturesUI::set_images ( IplImage * queryImage, IplImage * resultImage )
{
  int w1 = queryImage ? queryImage->width : 0;
  int w2 = resultImage ? resultImage->width : 0;
  int h1 = queryImage ? queryImage->height : 0;
  int h2 = resultImage ? resultImage->height : 0;
  int h = MAX ( h1, h2 );

  resize_windows ( w1, w2, h );
  queryView->setImage ( queryImage );
  resultView->setImage ( resultImage );
}

// Set the query and result feature set pointers.
void FeaturesUI::set_features ( FeatureSet * queryFeatures,
                                FeatureSet * resultFeatures )
{
  queryView->setFeatures ( queryFeatures );
  resultView->setFeatures ( resultFeatures );
}

// Get the current match type.
int FeaturesUI::get_match_type () const
{
  return doc->get_match_algorithm ();
}

// Called when the user chooses the "Load Query Image" menu item.
void FeaturesUI::cb_load_query_image ()
{
  QString name = QFileDialog::getOpenFileName (
    this,
    "Open File",
    "~",
    "Images (*.png *.bmp *.pgm *.ppm *.jpg)" );

  doc->load_query_image ( name.toStdString ().c_str () );
}

// Called when the user chooses to load normal query features.
void FeaturesUI::cb_load_query_features ()
{
  QString name = QFileDialog::getOpenFileName ( this, "Open File", "~", "*.f" );

  doc->load_query_features ( name.toStdString ().c_str (), false );
}

// Called when the user chooses to load SIFT query features.
void FeaturesUI::cb_load_query_features_sift ()
{
  QString name = QFileDialog::getOpenFileName ( this,
                                                "Open File",
                                                "~",
                                                "*.key" );

  doc->load_query_features ( name.toStdString ().c_str (), true );
}

// Called when the user chooses to load a normal database.
void FeaturesUI::cb_load_image_database ()
{
  QString name = QFileDialog::getOpenFileName ( this, "Open File", "~", "*.db" );

  std::cerr << name.toStdString () << std::endl;
  doc->load_image_database ( name.toStdString ().c_str (), false );
}

// Called when the user chooses to load a SIFT database.
void FeaturesUI::cb_load_image_database_sift ()
{
  QString name = QFileDialog::getOpenFileName ( this,
                                                "Open File",
                                                "~",
                                                "*.kdb" );

  doc->load_image_database ( name.toStdString ().c_str (), true );
}

// Called when the user chooses the "Exit" menu item.
void FeaturesUI::cb_exit ()
{
  close ();
}

// Called when the user chooses the "Select All Features" menu item.
void FeaturesUI::cb_select_all_features ()
{
  doc->select_all_query_features ();
}

// Called when the user chooses the "Deselect All Features" menu item.
void FeaturesUI::cb_deselect_all_features ()
{
  doc->deselect_all_query_features ();
}

// Called when the user chooses the "Toggle Features" menu item.
void FeaturesUI::cb_toggle_features ()
{
  queryView->toggleFeatures ();
  resultView->toggleFeatures ();
}

// Called when the user chooses the "Perform Query" menu item.
void FeaturesUI::cb_perform_query ()
{
  doc->perform_query ();
}

// Called when the user selects "Algorithm 1".
void FeaturesUI::cb_match_algorithm_1 ()
{
  doc->set_match_algorithm ( 1 );
}

// Called when the user selects "Algorithm 2".
void FeaturesUI::cb_match_algorithm_2 ()
{
  doc->set_match_algorithm ( 2 );
}

// Called when the user clicks the "About" menu item.
void FeaturesUI::cb_about ()
{
  QMessageBox::about ( this, "y helo thar", "Project 2 Features UI" );
}
