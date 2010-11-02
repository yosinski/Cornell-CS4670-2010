#ifndef FEATURES_DOC_H
#define FEATURES_DOC_H

#include <opencv/cv.h>
#include <QObject>
#include <QPoint>

class FeatureSet;
class ImageDatabase;
class FeaturesUI;

// The FeaturesDoc class controls the functionality of the project, and
// has methods for all major operations, like loading image and
// features, and performing queries.
class FeaturesDoc : public QObject {
Q_OBJECT

private:
IplImage * queryImage;
FeatureSet * queryFeatures;

ImageDatabase * db;

IplImage * resultImage;

int matchType;

IplImage * loadImage ( const char * );
QPoint applyHomography ( CvMat *, int, int );
public:
FeaturesUI * ui;

signals:
void alert ( const QString & );

public:
// Create a new document.
FeaturesDoc ( FeaturesUI * );

// Destroy the document.
~FeaturesDoc()
{
}

// Load an image, feature set, or database.
void load_query_image ( const char * name );
void load_query_features ( const char * name, bool sift );
void load_image_database ( const char * name, bool sift );

// Perform a query on the currently loaded image and database.
void perform_query ();

// Select or deselect all query features.
void select_all_query_features ();
void deselect_all_query_features ();

// Set the match algorithm.
int get_match_algorithm ()
{
  return matchType;
}
void set_match_algorithm ( int type );
};

#endif
