#include <cassert>
#include <fstream>
#include "Features.h"
#include "FeaturesUI.h"
#ifdef Q_WS_MAEMO_5
 #include "FeaturesMaemo.h"
#endif
#include "FeaturesDoc.h"
#include <opencv/highgui.h>
#include <exception>
#include <QApplication>

void saveRocFile ( const char *       filename,
                   vector<float> &    thresholdList,
                   vector<ROCPoint> & results );
void saveAUCFile ( const char *       filename,
                   vector<float> &    thresholdList,
                   vector<ROCPoint> & results );

void convertToFloatImage ( IplImage * byteImage, IplImage * floatImage )
{
  cvConvertScale ( byteImage, floatImage, 1 / 255., 0 );
}

class FeatureError : public std::exception
{
std::string message;
public:
FeatureError ( const std::string & msg, const std::string & file )
{
  std::stringstream ss;

  ss << msg << " " << file;
  message = msg.c_str ();
}
~FeatureError () throw ( )
{
}
};

IplImage * loadByteImage ( const char * name )
{
  IplImage * image = cvLoadImage ( name );

  if ( !image )
  {
    return 0;
  }
  cvCvtColor ( image, image, CV_BGR2RGB );
  return image;
}

IplImage * loadFloatImage ( const char * name )
{
  IplImage * image = cvLoadImage ( name );

  if ( !image )
  {
    return 0;
  }
  cvCvtColor ( image, image, CV_BGR2RGB );
  IplImage * floatingImage = cvCreateImage ( cvGetSize ( image ),
                                             IPL_DEPTH_32F, 3 );
  cvConvertScale ( image, floatingImage, 1 / 255., 0 );
  cvReleaseImage ( &image );
  return floatingImage;
}

// Compute the features for a single image.
int mainComputeFeatures ( int argc, char * * argv )
{
  if ( ( argc < 4 ) || ( argc > 5 ) )
  {
    printf ( "usage: %s computeFeatures imagefile featurefile [featuretype]\n",
             argv[0] );

    return -1;
  }

  // Use feature type 1 as default.
  int type = 1;

  if ( argc > 4 )
  {
    type = atoi ( argv[4] );
  }

  IplImage * floatQueryImage = loadFloatImage ( argv[2] );

  if ( !floatQueryImage )
  {
    printf ( "couldn't load query image\n" );
    return -1;
  }

  // Compute the image features.
  FeatureSet features;
  computeFeatures ( floatQueryImage, features, type );

  // Save the image features.
  features.save ( argv[3] );

  return 0;
}

// Match the features of one image to another, the output file matches to a file
int mainMatchFeatures ( int argc, char * * argv )
{
  if ( ( argc < 6 ) || ( argc > 7 ) )
  {
    printf (
      "usage: %s matchFeatures featurefile1 featurefile2 threshold matchfile [matchtype]\n",
      argv[0] );
    return -1;
  }

  // Use match type 1 as default.
  int type = 1;

  if ( argc > 6 )
  {
    type = atoi ( argv[6] );
  }

  FeatureSet f1;
  FeatureSet f2;

  if ( !f1.load ( argv[2] ) )
  {
    printf ( "couldn't load feature file %s\n", argv[2] );
    return -1;
  }

  if ( !f2.load ( argv[3] ) )
  {
    printf ( "couldn't load feature file %s\n", argv[3] );
    return -1;
  }

  float threshold = atof ( argv[4] );

  vector<FeatureMatch> matches;
  float totalScore;

  // Compute the match.
  if ( !matchFeatures ( f1, f2, matches, totalScore, type ) )
  {
    printf ( "matching failed, probably due to invalid match type\n" );
    return -1;
  }

  // Output the matches
  const char * matchFile = argv[5];
  std::ofstream f ( matchFile );

  // Count number of matches

  int num_matches = matches.size ();
  int num_good_matches = 0;
  for ( int i = 0; i < num_matches; i++ )
  {
    if ( matches[i].score < threshold )
    {
      num_good_matches++;
    }
  }

  f << num_good_matches << std::endl;

  for ( int i = 0; i < num_matches; i++ )
  {
    if ( matches[i].score < threshold )
    {
      f << matches[i].id1 << " " << matches[i].id2 << " " <<
      matches[i].score << std::endl;
    }
  }

  f.close ();

  return 0;
}

// Match the features of one image to another, the output file matches to a file
int mainMatchSIFTFeatures ( int argc, char * * argv )
{
  if ( ( argc < 6 ) || ( argc > 7 ) )
  {
    printf (
      "usage: %s matchFeatures featurefile1 featurefile2 threshold matchfile [matchtype]\n",
      argv[0] );
    return -1;
  }

  // Use match type 1 as default.
  int type = 1;

  if ( argc > 6 )
  {
    type = atoi ( argv[6] );
  }

  FeatureSet f1;
  FeatureSet f2;

  if ( !f1.load_sift ( argv[2] ) )
  {
    printf ( "couldn't load feature file %s\n", argv[2] );
    return -1;
  }

  if ( !f2.load_sift ( argv[3] ) )
  {
    printf ( "couldn't load feature file %s\n", argv[3] );
    return -1;
  }

  float threshold = atof ( argv[4] );

  vector<FeatureMatch> matches;
  float totalScore;

  // Compute the match.
  if ( !matchFeatures ( f1, f2, matches, totalScore, type ) )
  {
    printf ( "matching failed, probably due to invalid match type\n" );
    return -1;
  }

  // Output the matches
  const char * matchFile = argv[5];
  std::ofstream f ( matchFile );

  // Count number of matches

  int num_matches = matches.size ();
  int num_good_matches = 0;
  for ( int i = 0; i < num_matches; i++ )
  {
    if ( matches[i].score < threshold )
    {
      num_good_matches++;
    }
  }

  f << num_good_matches << std::endl;
  for ( int i = 0; i < num_matches; i++ )
  {
    if ( matches[i].score < threshold )
    {
      f << matches[i].id1 << " " << matches[i].id2 << " " <<
      matches[i].score << std::endl;
    }
  }

  f.close ();

  return 0;
}

// Match the features of one image to another, then compare the match
// with a ground truth homography.
int mainTestMatch ( int argc, char * * argv )
{
  if ( ( argc < 5 ) || ( argc > 6 ) )
  {
    printf (
      "usage: %s testMatch featurefile1 featurefile2 homographyfile [matchtype]\n",
      argv[0] );

    return -1;
  }

  // Use feature type 1 as default.
  int type = 1;

  if ( argc > 5 )
  {
    type = atoi ( argv[5] );
  }

  FeatureSet f1;
  FeatureSet f2;

  if ( !f1.load ( argv[2] ) )
  {
    printf ( "couldn't load feature file %s\n", argv[2] );
    return -1;
  }

  if ( !f2.load ( argv[3] ) )
  {
    printf ( "couldn't load feature file %s\n", argv[3] );
    return -1;
  }

  float h[9];

  ifstream is ( argv[4] );

  if ( !is.is_open () )
  {
    printf ( "couldn't open homography file %s\n", argv[4] );
    return -1;
  }

  // Read in the homography matrix.
  is >> h[0] >> h[1] >> h[2];
  is >> h[3] >> h[4] >> h[5];
  is >> h[6] >> h[7] >> h[8];

  vector<FeatureMatch> matches;
  float totalScore;

  // Compute the match.
  if ( !matchFeatures ( f1, f2, matches, totalScore, type ) )
  {
    printf ( "matching failed, probably due to invalid match type\n" );
    return -1;
  }

  float d = evaluateMatch ( f1, f2, matches, h );

  // The total error is the average SSD distance between a
  // (correctly) transformed feature point in the first image and its
  // matched feature point in the second image.
  printf ( "%f\n", d );

  return 0;
}

// Match the features of one image to another, then compare the match
// with a ground truth homography. Compute the ROC points for various thresholds.
int mainRocTestMatch ( int argc, char * * argv )
{
  if ( ( argc < 7 ) || ( argc > 8 ) )
  {
    printf (
      "usage: %s roc featurefile1 featurefile2 homographyfile [matchtype] rocfilename aucfilename\n",
      argv[0] );

    return -1;
  }

  // Use feature type 1 as default.
  int type = 1;

  const char * filename;
  const char * aucfilename;

  if ( argc == 8 )
  {
    type = atoi ( argv[5] );
    filename = argv[6];
    aucfilename = argv[7];
  }

  if ( argc == 7 )
  {
    filename = argv[5];
    aucfilename = argv[6];
  }


  FeatureSet f1;
  FeatureSet f2;

  if ( !f1.load ( argv[2] ) )
  {
    printf ( "couldn't load feature file %s\n", argv[2] );
    return -1;
  }

  if ( !f2.load ( argv[3] ) )
  {
    printf ( "couldn't load feature file %s\n", argv[3] );
    return -1;
  }

  float h[9];

  ifstream is ( argv[4] );

  if ( !is.is_open () )
  {
    printf ( "couldn't open homography file %s\n", argv[4] );
    return -1;
  }

  // Read in the homography matrix.
  is >> h[0] >> h[1] >> h[2];
  is >> h[3] >> h[4] >> h[5];
  is >> h[6] >> h[7] >> h[8];

  vector<FeatureMatch> matches;
  vector<bool> isMatch;
  float totalScore;
  float maxDistance = 0;

  // Compute the match.
  if ( !matchFeatures ( f1, f2, matches, totalScore, type ) )
  {
    printf ( "matching failed, probably due to invalid match type\n" );
    return -1;
  }

  //float d = evaluateMatch(f1, f2, matches, h);
  addRocData ( f1, f2, matches, h, isMatch, 5, maxDistance );

  vector<float> thresholdList;

  for ( int i = 0; i < 102; i++ )
  {
    thresholdList.push_back ( maxDistance / 100.0 * i );
  }

  vector<ROCPoint> results = computeRocCurve ( matches, isMatch, thresholdList );

  // Write the ROC data to a vile
  saveRocFile ( filename, thresholdList, results );

  printf ( "\nroc file complete.\n" );

  // Write the AUC data to a file
  saveAUCFile ( aucfilename, thresholdList, results );

  printf ( "\nauc file complete.\n" );

  return 0;
}

// Match the SIFT features of one image to another, then compare the
// match with a ground truth homography.
int mainTestSIFTMatch ( int argc, char * * argv )
{
  if ( ( argc < 5 ) || ( argc > 6 ) )
  {
    printf (
      "usage: %s testSIFTMatch featurefile1 featurefile2 homographyfile [matchtype]\n",
      argv[0] );

    return -1;
  }

  // Use feature type 1 as default.
  int type = 1;

  if ( argc > 5 )
  {
    type = atoi ( argv[5] );
  }

  FeatureSet f1;
  FeatureSet f2;

  if ( !f1.load_sift ( argv[2] ) )
  {
    printf ( "couldn't load feature file %s\n", argv[2] );
    return -1;
  }

  if ( !f2.load_sift ( argv[3] ) )
  {
    printf ( "couldn't load SIFT feature file %s\n", argv[3] );
    return -1;
  }

  float h[9];

  ifstream is ( argv[4] );

  if ( !is.is_open () )
  {
    printf ( "couldn't open homography file %s\n", argv[4] );
    return -1;
  }

  // Read in the homography matrix.
  is >> h[0] >> h[1] >> h[2];
  is >> h[3] >> h[4] >> h[5];
  is >> h[6] >> h[7] >> h[8];

  vector<FeatureMatch> matches;
  float totalScore;

  // Compute the match.
  if ( !matchFeatures ( f1, f2, matches, totalScore, type ) )
  {
    printf ( "matching failed, probably due to invalid match type\n" );
    return -1;
  }

  float d = evaluateMatch ( f1, f2, matches, h );

  // The total error is the average SSD distance between a
  // (correctly) transformed feature point in the first image and its
  // matched feature point in the second image.
  printf ( "%f\n", d );

  return 0;
}

// Match the SIFT features of one image to another, then compare the
// match with a ground truth homography.  Compute the ROC points for various thresholds.
int mainRocTestSIFTMatch ( int argc, char * * argv )
{
  if ( ( argc < 7 ) || ( argc > 8 ) )
  {
    printf (
      "usage: %s rocSIFT featurefile1 featurefile2 homographyfile [matchtype] rocfilename aucfilename\n",
      argv[0] );

    return -1;
  }

  // Use feature type 1 as default.
  int type = 1;
  const char * filename;
  const char * aucfilename;

  if ( argc == 8 )
  {
    type = atoi ( argv[5] );
    filename = argv[6];
    aucfilename = argv[7];
  }

  if ( argc == 7 )
  {
    filename = argv[5];
    aucfilename = argv[6];
  }

  FeatureSet f1;
  FeatureSet f2;

  if ( !f1.load_sift ( argv[2] ) )
  {
    printf ( "couldn't load feature file %s\n", argv[2] );
    return -1;
  }

  if ( !f2.load_sift ( argv[3] ) )
  {
    printf ( "couldn't load SIFT feature file %s\n", argv[3] );
    return -1;
  }

  float h[9];

  ifstream is ( argv[4] );

  if ( !is.is_open () )
  {
    printf ( "couldn't open homography file %s\n", argv[4] );
    return -1;
  }

  // Read in the homography matrix.
  is >> h[0] >> h[1] >> h[2];
  is >> h[3] >> h[4] >> h[5];
  is >> h[6] >> h[7] >> h[8];

  vector<FeatureMatch> matches;
  vector<bool> isMatch;
  float maxDistance = 0.0;
  float totalScore;

  // Compute the match.
  if ( !matchFeatures ( f1, f2, matches, totalScore, type ) )
  {
    printf ( "matching failed, probably due to invalid match type\n" );
    return -1;
  }

  //float d = evaluateMatch(f1, f2, matches, h);
  addRocData ( f1, f2, matches, h, isMatch, 5, maxDistance );



  vector<float> thresholdList;

  for ( int i = 0; i < 102; i++ )
  {
    thresholdList.push_back ( maxDistance / 100.0 * i );
  }


  vector<ROCPoint> results = computeRocCurve ( matches, isMatch, thresholdList );

  // Write the ROC data to a vile
  saveRocFile ( filename, thresholdList, results );

  printf ( "\nSIFT roc file complete.\n" );

  // Write the AUC data to a file
  saveAUCFile ( aucfilename, thresholdList, results );

  printf ( "\nSIFT auc file complete.\n" );


  return 0;
}

int mainPanorama ( int argc, char * * argv )
{
  if ( argc < 5 || argc > 7 )
  {
    printf (
      "usage: %s panorama imageFile1 imageFile2 outputImageFile [featureType] [matchType]\n",
      argv[0] );
    return -1;
  }
  IplImage * img1 = loadFloatImage ( argv[2] );
  if ( !img1 )
  {
    return -1;
  }
  IplImage * img2 = loadFloatImage ( argv[3] );
  if ( !img2 )
  {
    return -1;
  }
  int featureType = 0;
  int matchType = 0;
  if ( argc > 5 )
  {
    featureType = atoi ( argv[5] );
  }
  if ( !featureType )
  {
    featureType = 3;
  }
  if ( argc > 6 )
  {
    matchType = atoi ( argv[6] );
  }
  if ( !matchType )
  {
    matchType = 2;
  }
  IplImage * output = constructPanorama ( img1, img2, featureType, matchType );
  cvReleaseImage ( &img1 );
  cvReleaseImage ( &img2 );
  cvConvertScale ( output, output, 255, 0 );
  cvCvtColor ( output, output, CV_RGB2BGR );
  cvSaveImage ( argv[4], output );
  cvReleaseImage ( &output );
  return 0;
}

// Compute the features of all the images in one of the benchmark sets,
// then match the first image in the set with all of the others,
// comparing the resulting match with the ground truth homography.
int mainBenchmark ( int argc, char * * argv )
{
  if ( ( argc != 3 ) && ( argc != 5 ) )
  {
    printf ( "usage: %s benchmark imagedir [featuretype matchtype]\n", argv[0] );
    return -1;
  }

  int featureType = 1;
  int matchType = 1;


  if ( argc == 5 )
  {
    featureType = atoi ( argv[3] );
    matchType = atoi ( argv[4] );
  }

  // Get the directory containing the images.
  string imageDir ( argv[2] );

  if ( ( imageDir[imageDir.size () - 1] != '/' ) &&
       ( imageDir[imageDir.size () - 1] != '\\' ) )
  {
    imageDir += '/';
  }

  string imageFile;
  FeatureSet features[6];

  // Compute the features for each of the six images in the set.
  for ( int i = 0; i < 6; i++ )
  {
    imageFile = imageDir + "img" + ( char )( '1' + i ) + ".ppm";

    // Load the query image.
#if 1
    IplImage * floatImage = loadFloatImage ( imageFile.c_str () );

    if ( floatImage == NULL )
    {
      printf ( "couldn't load image %d\n", i + 1 );
      return -1;
    }

#else
    IplImage * floatImage = loadFloatImage ( imageFile.c_str () );
#endif

    // Compute the image features.
    printf ( "computing features for image %d\n", i + 1 );
    computeFeatures ( floatImage, features[i], featureType );
    cvReleaseImage ( &floatImage );
  }

  string homographyFile;
  float h[9];
  vector<FeatureMatch> matches;

  float d = 0;

  //AUC parameter
  float auc = 0;
  vector<bool> isMatch;
  float maxDistance = 0.0;

  // Match the first image with each of the other images, and compare
  // the results to the ground truth homography.
  for ( int i = 1; i < 6; i++ )
  {
    // Open the homography file.
    homographyFile = imageDir + "H1to" + ( char )( '1' + i ) + "p";

    ifstream is ( homographyFile.c_str () );

    if ( !is.is_open () )
    {
      printf ( "couldn't open homography file %s\n", homographyFile.c_str () );
      return -1;
    }

    // Read in the homography matrix.
    is >> h[0] >> h[1] >> h[2];
    is >> h[3] >> h[4] >> h[5];
    is >> h[6] >> h[7] >> h[8];

    is.close ();

    matches.clear ();
    isMatch.clear ();
    float totalScore;

    // Compute the match.
    printf ( "matching image 1 with image %d\n", i + 1 );
    if ( !matchFeatures ( features[0], features[i], matches, totalScore,
                          matchType ) )
    {
      printf ( "matching failed, probably due to invalid match type\n" );
      return -1;
    }

    d += evaluateMatch ( features[0], features[i], matches, h );

    //compute AUC
    addRocData ( features[0], features[i], matches, h, isMatch, 5, maxDistance );

    vector<float> thresholdList;

    for ( int i = 0; i < 102; i++ )
    {
      thresholdList.push_back ( maxDistance / 100.0 * i );
    }

    vector<ROCPoint> results = computeRocCurve ( matches,
                                                 isMatch,
                                                 thresholdList );
    float newAuc = computeAUC ( results );
    std::cerr << newAuc << std::endl;
    auc += newAuc;
  }

  printf ( "\naverage error: %f pixels\n", d / 5 );
  printf ( "\naverage AUC: %f\n", auc / 5 );

  return 0;
}


void saveRocFile ( const char *       filename,
                   vector<float> &    thresholdList,
                   vector<ROCPoint> & results )
{
  FILE * stream = fopen ( filename, "wt" );

  if ( stream == 0 )
  {
    throw FeatureError ( "saveRoc: could not open %s", filename );
  }

  // Write out the numbers

  fprintf ( stream, "Threshold\tFP Rate\tTP Rate\n" );
  for ( int i = 0; i < ( int )results.size (); i++ )
  {
    fprintf ( stream,
              "%lf\t%lf\t%lf\n",
              thresholdList[i],
              results[i].falseRate,
              results[i].trueRate );
  }

  if ( fclose ( stream ) )
  {
    throw FeatureError ( "saveRoc: error closing file", filename );
  }
}

void saveAUCFile ( const char *       filename,
                   vector<float> & /*(kmatzen) Unused var thresholdList*/,
                   vector<ROCPoint> & results )
{
  float auc;
  FILE * stream = fopen ( filename, "wt" );

  if ( stream == 0 )
  {
    throw FeatureError ( "saveAUC: could not open %s", filename );
  }

  auc = computeAUC ( results );
  fprintf ( stream, "%lf\n", auc );
  if ( fclose ( stream ) )
  {
    throw FeatureError ( "saveAUC: error closing file", filename );
  }
}

int main ( int argc, char * * argv )
{
  if ( argc > 1 )
  {
    if ( strcmp ( argv[1], "computeFeatures" ) == 0 )
    {
      return mainComputeFeatures ( argc, argv );
    }
    else if ( strcmp ( argv[1], "matchFeatures" ) == 0 )
    {
      return mainMatchFeatures ( argc, argv );
    }
    else if ( strcmp ( argv[1], "matchSIFTFeatures" ) == 0 )
    {
      return mainMatchSIFTFeatures ( argc, argv );
    }
    else if ( strcmp ( argv[1], "testMatch" ) == 0 )
    {
      return mainTestMatch ( argc, argv );
    }
    else if ( strcmp ( argv[1], "testSIFTMatch" ) == 0 )
    {
      return mainTestSIFTMatch ( argc, argv );
    }
    else if ( strcmp ( argv[1], "benchmark" ) == 0 )
    {
      return mainBenchmark ( argc, argv );
    }
    else if ( strcmp ( argv[1], "rocSIFT" ) == 0 )
    {
      //return saveRoc(argc,argv);
      mainRocTestSIFTMatch ( argc, argv );
    }
    else if ( strcmp ( argv[1], "roc" ) == 0 )
    {
      //return saveRoc(argc,argv);
      mainRocTestMatch ( argc, argv );
    }
    else if ( strcmp ( argv[1], "panorama" ) == 0 )
    {
      mainPanorama ( argc, argv );
    }

    else
    {
      printf ( "usage:\n" );
      printf ( "\t%s\n", argv[0] );
      printf ( "\t%s computeFeatures imagefile featurefile [featuretype]\n",
               argv[0] );
      printf (
        "\t%s matchFeatures featurefile1 featurefile2 threshold matchfile [matchtype]\n",
        argv[0] );
      printf (
        "\t%s matchSIFTFeatures featurefile1 featurefile2 threshold matchfile [matchtype]\n",
        argv[0] );
      // printf("\t%s testMatch featurefile1 featurefile2 homographyfile [matchtype]\n", argv[0]);
      // printf("\t%s testSIFTMatch featurefile1 featurefile2 homographyfile [matchtype]\n", argv[0]);
      // printf("\t%s benchmark imagedir [featuretype matchtype]\n", argv[0]);
      printf (
        "\t%s rocSIFT featurefile1 featurefile2 homographyfile [matchtype] rocfilename aucfilename\n",
        argv[0] );
      printf (
        "\t%s roc featurefile1 featurefile2 homographyfile [matchtype] rocfilename aucfilename\n",
        argv[0] );
      printf ( "\t%s panorama imageFile1 imageFile2 outputImageFile\n",
               argv[0] );
      return -1;
    }
  }
  else
  {
    QApplication app ( argc, argv );
    // Use the GUI.
#ifdef Q_WS_MAEMO_5
    QWidget * ui = new FeaturesMaemo;
#else
    QWidget * ui = new FeaturesUI;
#endif
    app.setActiveWindow ( ui );
    ui->show ();
    return app.exec ();
  }
}
