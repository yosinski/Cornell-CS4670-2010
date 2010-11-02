#ifndef FeaturesUI_H
#define FeaturesUI_H

#include <QWidget>
#include <QMainWindow>
#include <QMenuBar>
#include <QMenu>
#include <opencv/cv.h>
#include "FeaturesDoc.h"
#include "ImageView.h"
#include <QPoint>

// The FeaturesUI class controls the UI.  Feel free to play around with
// the UI if you'd like.
class FeaturesUI : public QMainWindow {
Q_OBJECT

FeaturesDoc * doc;

ImageView * queryView;
ImageView * resultView;

public:
// Create the UI.
FeaturesUI ( QWidget * = 0 );

// Set the pointers to the two images.
void set_images ( IplImage * queryImage, IplImage * resultImage );

void set_box ( QPoint, QPoint, QPoint, QPoint );

// Set the pointers to the two feature sets.
void set_features ( FeatureSet * queryFeatures, FeatureSet * resultFeatures );
// Get the current match type.
int get_match_type () const;

private:
void resize_windows ( int w1, int w2, int h );
QMenu * createFileMenu ( QWidget * );
QMenu * createLoadQueryFeaturesMenu ( QWidget * );
QMenu * createLoadImageDatabaseMenu ( QWidget * );
QMenu * createImageMenu ( QWidget * );
QMenu * createOptionsMenu ( QWidget * );
QMenu * createSelectMatchAlgorithmMenu ( QWidget * );
QMenu * createHelpMenu ( QWidget * );

// Here are the callback functions.
public slots:
void cb_load_query_image ();
void cb_load_query_features ();
void cb_load_query_features_sift ();
void cb_load_image_database ();
void cb_load_image_database_sift ();
void cb_exit ();
void cb_select_all_features ();
void cb_deselect_all_features ();
void cb_toggle_features ();
void cb_perform_query ();
void cb_match_algorithm_1 ();
void cb_match_algorithm_2 ();
void cb_about ();
};

#endif
