#include <assert.h>
#include "Features.h"
#include "FeaturesUI.h"
#include "FeaturesDoc.h"
#include <opencv/highgui.h>

// Create a new document.
FeaturesDoc::FeaturesDoc( FeaturesUI * ui ) : ui ( ui )
{
  queryImage = NULL;
  queryFeatures = NULL;

  db = NULL;

  resultImage = NULL;

  matchType = 1;
}

IplImage * FeaturesDoc::loadImage ( const char * name )
{
  IplImage * image = cvLoadImage ( name );

  if ( !image )
  {
    return 0;
  }
  cvCvtColor ( image, image, CV_BGR2RGB );
  return image;
}

// Load an image file for use as the query image.
void FeaturesDoc::load_query_image ( const char * name )
{
  ui->set_images ( NULL, NULL );
  ui->set_features ( NULL, NULL );

  // Delete the current query image.
  if ( queryImage != NULL )
  {
    cvReleaseImage ( &queryImage );;
    queryImage = NULL;
  }

  // Delete the current query image features.
  if ( queryFeatures != NULL )
  {
    delete queryFeatures;
    queryFeatures = NULL;
  }

  // Delete the current result image.
  if ( resultImage != NULL )
  {
    cvReleaseImage ( &resultImage );
    resultImage = NULL;
  }

  // Load the image.
  queryImage = loadImage ( name );

  if ( queryImage == NULL )
  {
    emit alert ( "Couldn't load image file" );
  }
  else
  {
    // Update the UI.
    ui->resize ( queryImage->width, queryImage->height );
    ui->set_images ( queryImage, NULL );
    //kmatzen
    ui->set_features ( queryFeatures, NULL );
    emit alert ( "Image loaded" );
  }
}

// Load a set of features for the query image.
void FeaturesDoc::load_query_features ( const char * name, bool sift )
{
  if ( queryImage == NULL )
  {
    emit alert ( "No query image loaded" );
  }
  else
  {
    ui->set_images ( queryImage, NULL );
    ui->set_features ( NULL, NULL );

    // Delete the current query image features.
    if ( queryFeatures != NULL )
    {
      delete queryFeatures;
      queryFeatures = NULL;
    }

    // Delete the current result image.
    if ( resultImage != NULL )
    {
      cvReleaseImage ( &resultImage );
      resultImage = NULL;
    }

    queryFeatures = new FeatureSet ();

    // Load the feature set.
    if ( ( ( !sift ) &&
           ( queryFeatures->load ( name ) ) ) ||
         ( ( sift ) && ( queryFeatures->load_sift ( name ) ) ) )
    {
      ui->set_features ( queryFeatures, NULL );
      emit alert ( "Features set" );
    }
    else
    {
      delete queryFeatures;
      queryFeatures = NULL;

      emit alert ( "Couldn't load feature data file" );
    }
  }
}

// Load an image database.
void FeaturesDoc::load_image_database ( const char * name, bool sift )
{
  ui->set_images ( queryImage, NULL );
  ui->set_features ( queryFeatures, NULL );

  // Delete the current database.
  if ( db != NULL )
  {
    delete db;
    db = NULL;
  }

  // Delete the current result image.
  if ( resultImage != NULL )
  {
    cvReleaseImage ( &resultImage );
    resultImage = NULL;
  }

  db = new ImageDatabase ();

  // Load the database.
  if ( !db->load ( name, sift ) )
  {
    delete db;
    db = NULL;

    emit alert ( "Couldn't load database" );
  }
  else
  {
    emit alert ( "Database loaded" );
  }
}

QPoint FeaturesDoc::applyHomography ( CvMat * h, int x, int y )
{
  std::cerr << x << " " << y << std::endl;
  for ( int i = 0; i < h->height; ++i )
  {
    for ( int j = 0; j < h->width; ++j )
    {
      std::cerr << CV_MAT_ELEM ( *h, float, i, j ) << " ";
    }
    std::cerr << std::endl;
  }
  float xPrime = CV_MAT_ELEM ( *h, float, 0, 0 ) * x + CV_MAT_ELEM ( *h,
                                                                     float,
                                                                     0,
                                                                     1 ) * y +
                 CV_MAT_ELEM ( *h, float, 0, 2 );
  float yPrime = CV_MAT_ELEM ( *h, float, 1, 0 ) * x + CV_MAT_ELEM ( *h,
                                                                     float,
                                                                     1,
                                                                     1 ) * y +
                 CV_MAT_ELEM ( *h, float, 1, 2 );
  float w = CV_MAT_ELEM ( *h, float, 2, 0 ) * x + CV_MAT_ELEM ( *h,
                                                                float,
                                                                2,
                                                                1 ) * y +
            CV_MAT_ELEM ( *h, float, 2, 2 );
  std::cerr << xPrime << " " << yPrime << " " << w << std::endl;
  QPoint prime ( xPrime / w, yPrime / w );
  return prime;
}

// Perform a query on the loaded database.
void FeaturesDoc::perform_query ()
{
  ui->set_images ( queryImage, NULL );
  ui->set_features ( queryFeatures, NULL );
  ui->repaint ();

  if ( queryImage == NULL )
  {
    emit alert ( "No query image loaded" );
  }
  else if ( queryFeatures == NULL )
  {
    emit alert ( "No query features loaded" );
  }
  else if ( db == NULL )
  {
    emit alert ( "No image database loaded" );
  }
  else
  {
    FeatureSet selectedFeatures;
    queryFeatures->get_selected_features ( selectedFeatures );

    if ( selectedFeatures.size () == 0 )
    {
      emit alert ( "No features selected" );
    }
    else
    {
      size_t index;
      vector<FeatureMatch> matches;
      float score;

      if ( !performQuery ( selectedFeatures, *db, index, matches, score,
                           ui->get_match_type () ) )
      {
        emit alert ( "Query failed" );
      }
      else
      {
        // Delete the current result image.
        if ( resultImage != NULL )
        {
          cvReleaseImage ( &resultImage );
          resultImage = NULL;
        }

        // Load the image.
        resultImage = loadImage ( ( *db )[index].name.c_str () );

        if ( resultImage == NULL )
        {
          emit alert ( "Couldn't load result image file" );
        }
        else
        {
          ( *db )[index].features.deselect_all ();
          ( *queryFeatures ).deselect_all ();

          // Select the matched features.
          for ( unsigned int i = 0; i < matches.size (); i++ )
          {
            ( *queryFeatures )[matches[i].id1 - 1].selected = true;
            ( *db )[index].features[matches[i].id2 - 1].selected = true;
          }

          ui->set_images ( queryImage, resultImage );
          ui->set_features ( queryFeatures, &( ( *db )[index].features ) );
          int width = resultImage->width;
          int height = resultImage->height;
          CvMat * h = ransacHomography ( *queryFeatures, db->at (
                                           index ).features, matches, 100, 5 );
          QPoint point1 = applyHomography ( h, 0, 0 );
          QPoint point2 = applyHomography ( h, width, 0 );
          QPoint point3 = applyHomography ( h, width, height );
          QPoint point4 = applyHomography ( h, 0, height );
          cvReleaseMat ( &h );
          ui->set_box ( point1, point2, point3, point4 );
          emit alert ( "Matching complete" );
        }
      }
    }
  }
}

// Select all query features.
void FeaturesDoc::select_all_query_features ()
{
  if ( queryFeatures == NULL )
  {
    emit alert ( "No query features loaded" );
  }
  else
  {
    queryFeatures->select_all ();
    ui->repaint ();
    emit alert ( "Features selected" );
  }
}

// Deselect all query features.
void FeaturesDoc::deselect_all_query_features ()
{
  if ( queryFeatures == NULL )
  {
    emit alert ( "No query features loaded" );
  }
  else
  {
    queryFeatures->deselect_all ();
    ui->repaint ();
    emit alert ( "Features deselected" );
  }
}

// Set the match algorithm.
void FeaturesDoc::set_match_algorithm ( int type )
{
  matchType = type;
  std::stringstream ss;
  ss << "Algorithm set to type ";
  ss << type;
  emit alert ( ss.str ().c_str () );
}
