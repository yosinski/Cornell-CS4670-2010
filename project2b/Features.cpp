#include <cassert>
#include <cmath>
#include <iostream>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <opencv/cxcore.h>
#include "Features.h"
#include <stdio.h>

// Forward declarations
void applyHomography ( float   x, float   y, float & xNew, float & yNew, float   h [9] );

#define square(x) ((x)*(x))
#define bugme BUGME(__FILE__, __LINE__)
void BUGME(string file, int line) {std::cout << " [" << file << ":" << line << "]" << endl;}

// get value at (possibly exterior) coordinates
inline CvScalar safeVal (const IplImage * img, int ii, int jj)
{
  //assert ( img->depth == IPL_DEPTH_32F );
  //assert ( img->nChannels == 1 );

  // compute coordinates as if image was reflected across all borders.
  // Edge row is not reflected.
  ii = (ii >= 0     ? ii : -ii);
  ii = (ii < img->height ? ii : 2*img->height-ii-2);
  jj = (jj >= 0     ? jj : -jj);
  jj = (jj < img->width  ? jj : 2*img->width -jj-2);

  //return CV_IMAGE_ELEM(img, float, ii, jj);
  return cvGet2D(img, ii, jj);
}

// set value at (possibly exterior) coordinates
inline void safeSet (IplImage * img, int ii, int jj, CvScalar val)
{
  //assert ( img->depth == IPL_DEPTH_32F );
  //assert ( img->nChannels == 1 );

  // compute coordinates as if image was reflected across all borders.
  // Edge row is not reflected.
  ii = (ii >= 0     ? ii : -ii);
  ii = (ii < img->height ? ii : 2*img->height-ii-2);
  jj = (jj >= 0     ? jj : -jj);
  jj = (jj < img->width  ? jj : 2*img->width -jj-2);

  //return CV_IMAGE_ELEM(img, float, ii, jj);
  cvSet2D(img, ii, jj, val);
}


float test[9] = {1.0, 0.0, 280.0,
		0.0, 1.0, 0.0,
		0.0, 0.0, 1.0};

// This is the entry point for all panorama generation.  The output image will
// be allocated by your code and in particular should be allocated from a call
// to compositeImages.  This function will also depend on ransacHomography
// in order to compute a best homography for the pair of images.  You should
// use computeFeatures and matchFeatures when necessary.
IplImage * constructPanorama ( IplImage *     img1,
                               IplImage *     img2,
                               int            featureType,
                               int matchType
#ifdef                                        Q_WS_MAEMO_5
                               ,
                               Progressable * thread
#endif
                               )
{
  assert ( img1->depth == IPL_DEPTH_32F );
  assert ( img1->nChannels == 3 );
  assert ( img2->depth == IPL_DEPTH_32F );
  assert ( img2->nChannels == 3 );
  // @@@ TODO Project 2b
  //assert ( 0 ); // Remove this when ready

  FeatureSet features1;
  FeatureSet features2;

  computeFeatures(img1, features1, featureType
#ifdef Q_WS_MAEMO_5
		                  , thread
#endif
		                  );

  computeFeatures(img2, features2, featureType
#ifdef Q_WS_MAEMO_5
		                  , thread
#endif  
		                  );

  vector<FeatureMatch> matches;
  float totalScore = 0.0;
  matchFeatures(features1, features2, matches, totalScore, matchType
#ifdef Q_WS_MAEMO_5
		                  , thread
#endif  
		                  );

  CvMat* h = ransacHomography(features1, features2, matches, 1000, 10
#ifdef Q_WS_MAEMO_5
		                  , thread
#endif  
		                  );

  float* data = h->data.fl;
  printf("Final Homography Matrix:\n");
  printf(" %3f, %3f, %3f\n", data[0], data[1], data[2]);
  printf(" %3f, %3f, %3f\n", data[3], data[4], data[5]);
  printf(" %3f, %3f, %3f\n", data[6], data[7], data[8]);
  printf("     %3f should be rightmost, center row\n", cvmGet(h, 1, 2));  

  h->data.fl = test;

  data = h->data.fl;
  printf("Final Final Homography Matrix:\n");
  printf(" %3f, %3f, %3f\n", data[0], data[1], data[2]);
  printf(" %3f, %3f, %3f\n", data[3], data[4], data[5]);
  printf(" %3f, %3f, %3f\n", data[6], data[7], data[8]);

  IplImage* result = compositeImages(img1, img2, h
#ifdef Q_WS_MAEMO_5
		                  , thread
#endif  
		                  );

  return result;
}

// Applies homography h onto point p and returns the result
/*CvPoint applyHomography(CvPoint p, 
			CvMat *h
#ifdef Q_WS_MAEMO_5
			, Progressable * thread
#endif
			)
{
  CvMat * in = cvMat(3, 1, CV_32FC1);
  CvMat * out = cvMat(3, 1, CV_32FC1);

  cvmSet(in, 0, 0, p.x);
  cvmSet(in, 1, 0, p.y);
  cvmSet(in, 2, 0, 1);

  cvMul(h, in, out);

  if(cvmGet(out, 2, 0) == 0.0) //bad things happen at inf
    return cvPoint(1e10, 1e10);
  return cvPoint( cvmGet(out, 0, 0) / cvmGet(out, 2, 0),
		  cvmGet(out, 1, 0) / cvmGet(out, 2, 0));
}*/

// img1 and img2 are color images that you want to make into a panorama by
// applying the homography, h, to img2.  This function needs to determine the
// size of the output image and allocate the memory for it.
IplImage * compositeImages ( IplImage *     img1,
                             IplImage *     img2,
                             CvMat *        h
#ifdef                                      Q_WS_MAEMO_5
                             ,
                             Progressable * thread
#endif
                             )
{
  assert ( img1->depth == IPL_DEPTH_32F );
  assert ( img1->nChannels == 3 );
  assert ( img2->depth == IPL_DEPTH_32F );
  assert ( img2->nChannels == 3 );

  IplImage *alpha1 = cvCreateImage(cvSize(img1->width, img1->height), IPL_DEPTH_32F, 1);
  IplImage *alpha2 = cvCreateImage(cvSize(img2->width, img2->height), IPL_DEPTH_32F, 1);

  for(int x = 0; x < alpha1->width; x++)
    for(int y = 0; y < alpha1->height; y++)
      cvSet2D(alpha1, y, x, cvScalar((min(min(min(x, y), alpha1->width - x), alpha1->height - y))));
  for(int x = 0; x < alpha2->width; x++)
    for(int y = 0; y < alpha2->height; y++)
      cvSet2D(alpha2, y, x, cvScalar((min(min(min(x, y), alpha2->width - x), alpha2->height - y))));
	      
  // @@@ TODO Project 2b
  //assert ( 0 ); // Remove this when ready
  //check transformation boundries
  CvPoint2D32f foffset = cvPoint2D32f(0.0, 0.0);
  CvPoint offset = cvPoint(0, 0);
  //allocate image based on boundries
  float minx = 0;
  float maxx = 0;
  float miny = 0;
  float maxy = 0;
  float xp, yp;
  CvPoint2D32f test = cvPoint2D32f(0.0, 0.0);
  applyHomography(0.0, 0.0, xp, yp, (h->data.fl));
  test.x = xp; test.y = yp;
  minx = test.x < minx ? test.x : minx;
  maxx = test.x > maxx ? test.x : maxx;
  miny = test.y < miny ? test.y : miny;
  maxy = test.y > maxy ? test.y : maxy;
  applyHomography(0, img2->height-1, xp, yp, h->data.fl);
  test.x = xp; test.y = yp;
  minx = test.x < minx ? test.x : minx;
  maxx = test.x > maxx ? test.x : maxx;
  miny = test.y < miny ? test.y : miny;
  maxy = test.y > maxy ? test.y : maxy;
  applyHomography(img2->width-1, 0, xp, yp, h->data.fl);
  test.x = xp; test.y = yp;
  minx = test.x < minx ? test.x : minx;
  maxx = test.x > maxx ? test.x : maxx;
  miny = test.y < miny ? test.y : miny;
  maxy = test.y > maxy ? test.y : maxy;
  applyHomography(img2->width-1, img2->height-1, xp, yp, h->data.fl);
  test.x = xp; test.y = yp;
  minx = test.x < minx ? test.x : minx;
  maxx = test.x > maxx ? test.x : maxx;
  miny = test.y < miny ? test.y : miny;
  maxy = test.y > maxy ? test.y : maxy;
  if(minx < 0)
    foffset.x += fabs(minx-1);
  if(miny < 0)
    foffset.y += fabs(miny-1);
  if(maxx < img1->width)
    maxx = img1->width;
  if(maxy < img1->height)
    maxy = img1->height;
  
  IplImage* compImg = cvCreateImage(cvSize(ceil(maxx + foffset.x)+10, ceil(maxy + foffset.y)+10), IPL_DEPTH_32F, 3);
  IplImage* alpha3 =  cvCreateImage(cvSize(ceil(maxx + foffset.x)+10, ceil(maxy + foffset.y)+10), IPL_DEPTH_32F, 1);
  cvSet(compImg, cvScalar(0,0,0));
  cvSet(alpha3, cvScalar(0));

  // copy to int version
  offset.x = foffset.x + 5;
  offset.y = foffset.y + 5;

  CvRect area = cvRect(offset.x, offset.y, img1->width, img1->height);
  //copy img1 into new image
  cvSetImageROI(compImg, area);
  cvSetImageROI(alpha3, area);
  cvCopyImage(img1, compImg);
  cvCopyImage(alpha1, alpha3);
  cvResetImageROI(alpha3);
  cvResetImageROI(compImg);

  // [JBY] This might not work, definitely need to add alpha blending / feathering

  //transform img2 into new image
  for(int x = 0; x < img2->width; x++)
    for(int y = 0; y < img2->height; y++)
    {
      applyHomography(x, y, xp, yp, h->data.fl);
      
      //float alpha = cvGet2D(alpha3, yp+offset.y, xp+offset.x).val[0];
      float alpha = safeVal(alpha3, yp+offset.y, xp+offset.x).val[0];
      if(alpha > 0.0) //apply feathering if overlap occurs
      {
	float a2 = cvGet2D(alpha2, y, x).val[0];
	CvScalar RGB1 = cvGet2D(compImg, yp+offset.y, xp+offset.x);
	CvScalar RGB2 = cvGet2D(img2, y, x);
	CvScalar result = cvScalar(((RGB1.val[0] * alpha) + (RGB2.val[0] * a2))/(alpha + a2),
				   ((RGB1.val[1] * alpha) + (RGB2.val[1] * a2))/(alpha + a2),
				   ((RGB1.val[2] * alpha) + (RGB2.val[2] * a2))/(alpha + a2));
        cvSet2D(compImg, yp + offset.y, xp + offset.x, result);
      }
      else
      {
        //cvSet2D(compImg, yp + offset.y, xp + offset.x, cvGet2D(img2, y, x));
        //cvSet2D(alpha3, yp + offset.y, xp + offset.x, cvGet2D(alpha2, y, x));
        //cvSet2D(compImg, yp + offset.y, xp + offset.x, safeVal(img2, y, x));
        //cvSet2D(alpha3, yp + offset.y, xp + offset.x, safeVal(alpha2, y, x));    
        safeSet(compImg, yp + offset.y, xp + offset.x, safeVal(img2, y, x));
        safeSet(alpha3, yp + offset.y, xp + offset.x, safeVal(alpha2, y, x));    
      }
    }
  
  //cleanup
  cvReleaseImage(&alpha1);
  cvReleaseImage(&alpha2);
  cvReleaseImage(&alpha3);

  //return new image
  return compImg;
}

// Compute features of an image.
bool computeFeatures ( IplImage *     image,
                       FeatureSet &   features,
                       int featureType
#ifdef \
                       Q_WS_MAEMO_5
                       ,
                       Progressable * thread
#endif
                       )
{
  assert ( image->depth == IPL_DEPTH_32F );
  assert ( image->nChannels == 3 );
  // Instead of calling dummyComputeFeatures, write your own
  // feature computation routines and call them here.
  switch ( featureType )
  {
  case 1:
    dummyComputeFeatures ( image, features
#ifdef Q_WS_MAEMO_5
                           , thread
#endif
                           );
    break;
  case 2:
    ComputeMOPSFeatures ( image, features
#ifdef Q_WS_MAEMO_5
                          , thread
#endif
                          );
    break;
  case 3:
    ComputeSURFFeatures ( image, features
#ifdef Q_WS_MAEMO_5
                          , thread
#endif
                          );
    break;
  default:
    return false;
  }

  // This is just to make sure the IDs are assigned in order, because
  // the ID gets used to index into the feature array.
  for ( size_t i = 0; i < features.size (); ++i )
  {
    features [i].id = i + 1;
  }

  return true;
}

void ComputeSURFFeatures ( IplImage * image, FeatureSet & features
#ifdef Q_WS_MAEMO_5
                           , Progressable * thread
#endif
                           )
{
  cv::SURF surf;

  std::vector<cv::KeyPoint> keypoints;
  std::vector<float> descriptors;
  IplImage * temp = cvCreateImage ( cvGetSize ( image ), IPL_DEPTH_8U, 3 );
  cvConvertScale ( image, temp, 255, 0 );
  IplImage * temp2 = cvCreateImage ( cvGetSize ( image ), IPL_DEPTH_8U, 1 );
  cvCvtColor ( temp, temp2, CV_RGB2GRAY );
  cvReleaseImage ( &temp );
  surf ( temp2, cv::Mat (), keypoints, descriptors );
  cvReleaseImage ( &temp );
  features.clear ();
  features.reserve ( keypoints.size () );
  for ( size_t i = 0; i < keypoints.size (); ++i )
  {
#ifdef Q_WS_MAEMO_5
    if ( thread && i % ( keypoints.size () / 100 ) == 0 )
    {
      thread->emitProgressUpdate ( 100 * i / keypoints.size () );
    }
#endif
    Feature f;
    f.x = keypoints[i].pt.x;
    f.y = keypoints[i].pt.y;
    f.type = 3;
    f.id = 0;
    f.angleRadians = keypoints[i].angle;
    f.data.reserve ( surf.descriptorSize () );
    for ( int j = 0; j < surf.descriptorSize (); ++j )
    {
      f.data.push_back ( descriptors [i * surf.descriptorSize () + j] );
    }
    features.push_back ( f );
  }
}

// Perform a query on the database.  This simply runs matchFeatures on
// each image in the database, and returns the feature set of the best
// matching image.
bool performQuery ( const FeatureSet &     f,
                    const ImageDatabase &  db,
                    size_t &               bestIndex,
                    vector<FeatureMatch> & bestMatches,
                    float &
                    bestScore,
                    int matchType
#ifdef                                     Q_WS_MAEMO_5
                    ,
                    Progressable *         thread
#endif
                    )
{
  bestScore = 1e100;
  bestIndex = 0;


  for ( size_t i = 0; i < db.size (); ++i )
  {
    vector<FeatureMatch> tempMatches;
    float tempScore = 0;
    if ( !matchFeatures ( f, db [i].features, tempMatches, tempScore,
                          matchType
#ifdef Q_WS_MAEMO_5
                          , thread
#endif
                          ) )
    {
      return false;
    }

    if ( tempScore < bestScore )
    {
      bestIndex = i;
      bestScore = tempScore;
      bestMatches = tempMatches;
    }
  }
  return true;
}

// Match one feature set with another.
bool matchFeatures ( const FeatureSet &     f1,
                     const FeatureSet &     f2,
                     vector<FeatureMatch> & matches,
                     float &                totalScore,
                     int matchType
#ifdef                                      Q_WS_MAEMO_5
                     ,
                     Progressable *         thread
#endif
                     )
{
  // We have given you the ssd matching function, you must write your own
  // feature matching function for the ratio test.

  std::cout << std::endl << "Matching features, type " << matchType << "......." << std::endl;

  switch ( matchType )
  {
  case 1:
    ssdMatchFeatures ( f1, f2, matches, totalScore
#ifdef Q_WS_MAEMO_5
                       , thread
#endif
                       );
    return true;
  case 2:
    ratioMatchFeatures ( f1, f2, matches, totalScore
#ifdef Q_WS_MAEMO_5
                         , thread
#endif
                         );
    return true;
  default:
    return false;
  }
}

// Evaluate a match using a ground truth homography.  This computes the
// average SSD distance between the matched feature points and
// the actual transformed positions.
float evaluateMatch ( const FeatureSet &           f1,
                      const FeatureSet &           f2,
                      const vector<FeatureMatch> & matches,
                      float                        h [9] )
{
  float d = 0;
  int n = 0;

  float xNew;
  float yNew;

  size_t num_matches = matches.size ();

  for ( size_t i = 0; i < num_matches; ++i )
  {
    int id1 = matches [i].id1;
    int id2 = matches [i].id2;
    assert ( id1 );
    assert ( id2 );
    applyHomography ( f1 [id1 - 1].x, f1 [id1 - 1].y, xNew, yNew, h );
    d +=
      sqrt ( pow ( xNew - f2 [id2 - 1].x,
                   2 ) + pow ( yNew - f2 [id2 - 1].y, 2 ) );
    ++n;
  }

  return d / n;
}

void addRocData ( const FeatureSet &           f1,
                  const FeatureSet &           f2,
                  const vector<FeatureMatch> & matches,
                  float                        h [9],
                  vector<bool> &
                  isMatch,
                  float                        threshold,
                  float &
                  maxD )
{
  float d = 0;

  maxD = 0;
  float xNew;
  float yNew;

  size_t num_matches = matches.size ();

  for ( size_t i = 0; i < num_matches; ++i )
  {
    int id1 = matches [i].id1;
    int id2 = matches [i].id2;
    assert ( id1 );
    assert ( id2 );
    applyHomography ( f1 [id1 - 1].x, f1 [id1 - 1].y, xNew, yNew, h );

    // Ignore unmatched points.  There might be a better way to
    // handle this.
    d =
      sqrt ( pow ( xNew - f2 [id2 - 1].x,
                   2 ) + pow ( yNew - f2 [id2 - 1].y, 2 ) );
    if ( d <= threshold )
    {
      isMatch.push_back ( 1 );
    }
    else
    {
      isMatch.push_back ( 0 );
    }

    if ( matches [i].score > maxD )
    {
      maxD = matches [i].score;
    }
  }
}

vector<ROCPoint> computeRocCurve ( vector<FeatureMatch> & matches,
                                   vector<bool> &         isMatch,
                                   vector<float> &        thresholds )
{
  vector<ROCPoint> dataPoints;

  for ( size_t i = 0; i < thresholds.size (); ++i )
  {
    int tp = 0;
    int actualCorrect = 0;
    int fp = 0;
    int actualError = 0;
    int total = 0;

    size_t num_matches = matches.size ();
    for ( size_t j = 0; j < num_matches; ++j )
    {
      if ( isMatch [j] )
      {
        actualCorrect++;
        if ( matches [j].score < thresholds [i] )
        {
          tp++;
        }
      }
      else
      {
        actualError++;
        if ( matches [j].score < thresholds [i] )
        {
          fp++;
        }
      }
      total++;
    }

    ROCPoint newPoint;
    newPoint.trueRate = ( float( tp ) / actualCorrect );
    newPoint.falseRate = ( float( fp ) / actualError );

    dataPoints.push_back ( newPoint );
  }

  return dataPoints;
}


// Compute silly example features.  This doesn't do anything
// meaningful.
void dummyComputeFeatures ( IplImage * image, FeatureSet & features
#ifdef Q_WS_MAEMO_5
                            , Progressable * thread
#endif
                            )
{
  assert ( image->depth == IPL_DEPTH_32F );
  assert ( image->nChannels == 3 );

  Feature f;
#ifdef Q_WS_MAEMO_5
  int total_size = image->height * image->width;
#endif
  int count = 0;
  for ( int y = 0; y < image->height; ++y )
  {
    for ( int x = 0; x < image->width; ++x )
    {
// @@@ Nothing to do here, but take note of how you can notify the UI on the
// phone to display the status on a progress bar.  Values range from 0 - 100%
#ifdef Q_WS_MAEMO_5
      if ( thread && count % ( total_size / 100 ) == 0 )
      {
        thread->emitProgressUpdate ( 100 * count / total_size );
      }
#endif
      ++count;
      float r = CV_IMAGE_ELEM ( image, float, y, 3 * x );
      float g = CV_IMAGE_ELEM ( image, float, y, 3 * x + 1 );
      float b = CV_IMAGE_ELEM ( image, float, y, 3 * x + 2 );
      if ( ( int )( 255 * ( r + g + b ) + 0.5 ) % 100 == 1 )
      {
        // If the pixel satisfies this meaningless criterion,
        // make it a feature.

        f.type = 1;
        f.id += 1;
        assert ( f.id );
        f.x = x;
        f.y = y;

        f.data.resize ( 1 );
        f.data [0] = r + g + b;

        features.push_back ( f );
      }
    }
  }
}

void ComputeMOPSFeatures ( IplImage * image, FeatureSet & features
#ifdef Q_WS_MAEMO_5
                           , Progressable * thread
#endif
                           )
{
  assert ( image->depth == IPL_DEPTH_32F );
  assert ( image->nChannels == 3 );

  //Create grayscale image used for MOPS detection
  IplImage * grayImage =
    cvCreateImage ( cvGetSize ( image ), IPL_DEPTH_32F, 1 );
  cvCvtColor ( image, grayImage, CV_RGB2GRAY );

  //Create image to store MOPS values
  IplImage * harrisImage =
    cvCreateImage ( cvGetSize ( image ), IPL_DEPTH_32F, 1 );

  //Create image to store local maximum harris values as 255, other pixels 0
  IplImage * harrisMaxImage =
    cvCreateImage ( cvGetSize ( image ), IPL_DEPTH_32F, 1 );

  //TO DO--------------------------------------------------------------------
  //function puts harris values at each pixel position in harrisImage and
  // orientations at each pixel position in orientationImage
  IplImage * orientationImage =
    cvCreateImage ( cvGetSize ( image ), IPL_DEPTH_32F, 1 );
  //@@@PROJECT 2b: Fill in computeMOPSValues if you want to use MOPS
  computeMOPSValues ( grayImage, harrisImage, orientationImage );
  cvSaveImage ( "harris.png", harrisImage );

  //TO DO---------------------------------------------------------------------
  //Loop through harrisValues and find the best features in a local 3x3 maximum
  //compute the feature descriptor
  //@@@PROJECT 2b: Fill in computeLocalMaxima if you want to use MOPS
  computeLocalMaxima ( harrisImage, harrisMaxImage );
  cvSaveImage ( "harrisMax.png", harrisMaxImage );

  //TO DO--------------------------------------------------------------------
  //Loop through feature points in harrisMaxImage and create feature descriptor
  //for each point above a threshold
  //@@@PROJECT 2b: Copy your project 2a code here if you want to use MOPS


  cvReleaseImage ( &orientationImage );
  cvReleaseImage ( &harrisImage );
  cvReleaseImage ( &grayImage );
  cvReleaseImage ( &harrisMaxImage );
}

//TO DO---------------------------------------------------------------------
//Loop through the image to compute the harris corner values as described in
//class
// srcImage:  grayscale of original image
// harrisImage:  populate the harris values per pixel in this image
// orientationImage: populate the orientation in this image
void computeMOPSValues ( IplImage * srcImage,
                         IplImage * harrisImage,
                         IplImage * orientationImage )
{
  assert ( srcImage->depth == IPL_DEPTH_32F );
  assert ( harrisImage->depth == IPL_DEPTH_32F );
  assert ( orientationImage->depth == IPL_DEPTH_32F );
  assert ( srcImage->nChannels == 1 );
  assert ( harrisImage->nChannels == 1 );
  assert ( orientationImage->nChannels == 1 );

  //@@@PROJECT 2b: Copy your project 2a code here if you want to use MOPS
}

//Loop through the image to compute the harris corner values as described in
//class
// srcImage:  image with MOPS values
// destImage: Assign 1 to local maximum in 3x3 window, 0 otherwise
void computeLocalMaxima ( IplImage * srcImage, IplImage * destImage )
{
  assert ( srcImage->depth == IPL_DEPTH_32F );
  assert ( destImage->depth == IPL_DEPTH_32F );
  assert ( srcImage->nChannels == 1 );
  assert ( destImage->nChannels == 1 );

  //@@@PROJECT 2b: Copy your project 2a code here if you want to use MOPS
}


// Perform simple feature matching.  This just uses the SSD
// distance between two feature vectors, and matches a feature in the
// first image with the closest feature in the second image.  It can
// match multiple features in the first image to the same feature in
// the second image.
void ssdMatchFeatures ( const FeatureSet &     f1,
                        const FeatureSet &     f2,
                        vector<FeatureMatch> & matches,
                        float &                totalScore
#ifdef                                         Q_WS_MAEMO_5
                        ,
                        Progressable *         thread
#endif
                        )
{
  // @@@ You can leave is as-is since you will probably want to use the ratio
  // test anyway.  It is okay if you want to replace this with FLANN code such
  // as what we provide in the ratio test function.
  totalScore = 0;
#ifdef Q_WS_MAEMO_5
  int count = 0;
  int totalCount = f1.size ();
#endif
  for ( FeatureSet::const_iterator i1 = f1.begin (); i1 != f1.end (); ++i1 )
  {
#ifdef Q_WS_MAEMO_5
    if ( thread && count % ( totalCount / 100 ) == 0 )
    {
      thread->emitProgressUpdate ( 100 * count / totalCount );
    }
    ++count;
#endif
    size_t bestMatch = 1;
    float bestScore = 1e100;
    for ( FeatureSet::const_iterator i2 = f2.begin (); i2 != f2.end (); ++i2 )
    {
      float score = 0;
      assert ( i1->data.size () == i2->data.size () );
      for ( size_t i = 0; i < i1->data.size (); ++i )
      {
        float diff = i1->data[i] - i2->data[i];
        score += diff * diff;
      }
      if ( score < bestScore )
      {
        bestMatch = i2->id;
        bestScore = score;
      }
    }
    FeatureMatch m;
    m.id1 = i1->id;
    m.id2 = f2[bestMatch - 1].id;
    m.score = bestScore;
    totalScore += m.score;
    matches.push_back ( m );
  }
}

// This just uses the ratio of the SSD distance of the two best matches
// and matches a feature in the first image with the closest feature in the
// second image.
// It can match multiple features in the first image to the same feature in
// the second image.  (See class notes for more information)
void ratioMatchFeatures ( const FeatureSet &     f1,
                          const FeatureSet &     f2,
                          vector<FeatureMatch> & matches,
                          float &                totalScore
#ifdef                                           Q_WS_MAEMO_5
                          ,
                          Progressable *         thread
#endif
                          )
{
  // @@@ We are improving the speed of your matching code using the
  // Fast Library for Approximate Nearest Neighbors (FLANN)
  // We've done the work to populate the index.  Now you need to
  // look up how to query it and how to use the results.  Feel free
  // to use it for SSD match as well.
  cv::Mat features ( f2.size (), f2[0].data.size (), CV_32F );

  for ( size_t i = 0; i < f2.size (); ++i )
    {
      for ( size_t j = 0; j < f2[i].data.size (); ++j )
        {
          features.at<float>( ( int )i, ( int )j ) = f2[i].data[j];
        }
    }
  cv::flann::Index::Index flannIndex ( features, cv::flann::KDTreeIndexParams () );
  // End FLANN index population

  float threshold = 0.8;
  
  totalScore = 0;
  int count = 0;
#ifdef Q_WS_MAEMO_5
  int totalCount = f1.size ();
#endif
  vector<float> dists(2);
  vector<int> indicies(2);
  int i = 0;
  for ( FeatureSet::const_iterator i1 = f1.begin (); i1 != f1.end (); ++i1 )
    {
      flannIndex.knnSearch(i1->data, indicies, dists, 2, NULL);

      FeatureMatch fm;
      fm.id1 = i1->id;

      //matches[count].id1 = i1->id;
      if(dists[0] <= dists[1])
        {
          fm.id2 = f2[indicies[0]].id;
          fm.score = dists[0]/dists[1];
        }
      else
        {
          fm.id2 = f2[indicies[1]].id;
          fm.score = dists[1]/dists[0];
        }
      if(fm.score < threshold)
	{
	  totalScore += fm.score;
	  matches.push_back(fm);
	}

#ifdef Q_WS_MAEMO_5
      if ( thread && count % ( totalCount / 100 ) == 0 )
        {
          thread->emitProgressUpdate ( 100 * count / totalCount );
        }
#endif
      ++count;
      // @@@ Find out how to query a cv::flann::Index::Index
    }

  printf("totalScore is %f\n", totalScore);
}

// RANSAC as described in lecture.  The result is a 3x3 homography matrix that
// computeHomography produced.  computeHomography expects a preallocated 3x3
// matrix of type CV_32F.  Just a tip, if you want to calculate an exact
// homography for 4 point correspondences, it's easy to pass computeHomography
// the full f1 and f2 vectors and just construct a temporary FeatureMatch
// vector with four elements from the full matches vector.
CvMat * ransacHomography ( const std::vector<Feature> &      f1,
                           const std::vector<Feature> &      f2,
                           const std::vector<FeatureMatch> & matches,
                           int                               numRounds,
                           int inlierThreshold
#ifdef                                                       Q_WS_MAEMO_5
                           ,
                           Progressable *                    thread
#endif
                           )
{
  // @@@ TODO Project 2b

  int ii, jj, idx[4], tries;
  std::vector<FeatureMatch> fourMatches;
  CvMat * hh;
  CvMat * hBest = NULL;
  int inlierBest = 0;
  bugme; 
  srand(time(NULL));
  for (ii = 0; ii < numRounds; ++ii) {
    // 1. Pick 4 random matches
    for (tries = 0; tries < 1000; ++tries) {
      fourMatches.clear();
      for (jj = 0; jj < 4; ++jj) {
        idx[jj] = rand() % matches.size();
        fourMatches.push_back(matches[idx[jj]]);
      }
      if ((matches[idx[0]].id1 != matches[idx[1]].id1) &&
          (matches[idx[0]].id1 != matches[idx[2]].id1) &&
          (matches[idx[0]].id1 != matches[idx[3]].id1) &&
          (matches[idx[1]].id1 != matches[idx[2]].id1) &&
          (matches[idx[1]].id1 != matches[idx[3]].id1) &&
          (matches[idx[2]].id1 != matches[idx[3]].id1) &&
          (matches[idx[0]].id2 != matches[idx[1]].id2) &&
          (matches[idx[0]].id2 != matches[idx[2]].id2) &&
          (matches[idx[0]].id2 != matches[idx[3]].id2) &&
          (matches[idx[1]].id2 != matches[idx[2]].id2) &&
          (matches[idx[1]].id2 != matches[idx[3]].id2) &&
          (matches[idx[2]].id2 != matches[idx[3]].id2))
        break;   // good, matches were unique
      else
        printf("Had to try again... this should be rare!\n");
    }
    assert (tries != 1000);

    // 2. For this random match, compute the homography
    hh = computeHomography(f1, f2, fourMatches, hh);
    //printf("hh is %d\n", hh);
    //printf("hh->data.fl[0] is %d\n", hh->data.fl[0]);

    // 3. Transform all points via the computed homography and see how
    // many are inliers
    float xNew, yNew;
    int countInliers = 0, dist;
    for (vector<FeatureMatch>::const_iterator mm = matches.begin (); mm != matches.end (); ++mm) {
      applyHomography(f1[mm->id1].x, f1[mm->id1].y, xNew, yNew, hh->data.fl);
      
      dist = square(xNew-f2[mm->id2].x) + square(yNew-f2[mm->id2].y);
      
      //printf("dist %d vs inlierThreshold %d\n", dist, inlierThreshold);
      if (dist < inlierThreshold*inlierThreshold)
        ++countInliers;
    }
    
    if (countInliers > inlierBest) {
      // new best
      inlierBest = countInliers;
      if (hBest)
        cvReleaseMat(&hBest);
      hBest = cvCloneMat(hh);
      float* data = hBest->data.fl;
      printf("\"Best\" Homography Matrix:\n");
      printf(" %3f, %3f, %3f\n", data[0], data[1], data[2]);
      printf(" %3f, %3f, %3f\n", data[3], data[4], data[5]);
      printf(" %3f, %3f, %3f\n", data[6], data[7], data[8]);
      printf("     %3f should be rightmost, center row\n", cvmGet(hBest, 1, 2));
    } else {
      // not best, so release this homography matrix
      cvReleaseMat(&hh);
    }
    
  }

  printf("Number of RANSAC Inliers: %d\n", inlierBest);

  assert ( hBest );       // make sure we got something
  
  return hBest;
}

// The resulting matrix is a 3x3 homography matrix.  You may find cvSolve
// (CV_SVD option) useful for solving the least squares problem.  The memory
// for this matrix is allocated by this caller.
CvMat * computeHomography ( const std::vector<Feature> &      f1,
                            const std::vector<Feature> &      f2,
                            const std::vector<FeatureMatch> & matches,
                            CvMat *                           h )
{
  // @@@ TODO Project 2b
  //assert ( 0 ); // Remove when ready

  //allocate a 3x3 matrix
  h = cvCreateMat(3, 3, CV_32FC1);
  
  //Construct Matrix A (for At=b)
  //Construct Vector b (for At=b)
  CvMat* A = cvCreateMat(2*(matches.size()), 8, CV_32FC1);
  CvMat* b = cvCreateMat(2*(matches.size()), 1, CV_32FC1);
  int count = 0;
  for ( vector<FeatureMatch>::const_iterator i1 = matches.begin (); i1 != matches.end (); ++i1 )
  {
    float x = f1[i1->id1].x; //original point in first image
    float y = f1[i1->id1].y;
    float xp = f2[i1->id2].x; //target in second image
    float yp = f2[i1->id2].y;
    cvmSet(A, count, 0, x);
    cvmSet(A, count, 1, y);
    cvmSet(A, count, 2, 1);
    cvmSet(A, count, 3, 0);
    cvmSet(A, count, 4, 0);
    cvmSet(A, count, 5, 0);
    cvmSet(A, count, 6, -(x*xp));
    cvmSet(A, count, 7, -(y*xp));
    cvmSet(b, count, 0, xp);
    count++;
    cvmSet(A, count, 0, 0);
    cvmSet(A, count, 1, 0);
    cvmSet(A, count, 2, 0);
    cvmSet(A, count, 3, x);
    cvmSet(A, count, 4, y);
    cvmSet(A, count, 5, 1);
    cvmSet(A, count, 6, -(x*yp));
    cvmSet(A, count, 7, -(y*yp));
    cvmSet(b, count, 0, yp);
    count++;
  }

  //for(uint i = 0; i < 2*(matches.size()); i++) {
  //  cvmSet(b, i, 0, 0); //set all to 0
  //}

  printf(" A Matrix:\n");
  for ( int i = 0; i < 2*matches.size(); i++ )
  {
    printf("   | ");
    for (int j = 0; j < 8; j++)
      printf("%0.3f\t", cvmGet(A, i, j));
    printf("|\n");
  }

  printf(" B Vector:\n");
  for ( int i = 0; i < 2*matches.size(); i++ )
    printf("   | %0.3f \t|\n", cvmGet(b, i, 0));


  //Run Solve:
  CvMat* x = cvCreateMat(8, 1, CV_32FC1);
  cvSolve(A, b, x, CV_SVD);

  cvmSet(h, 0, 0, cvmGet(x, 0, 0));
  cvmSet(h, 0, 1, cvmGet(x, 1, 0));
  cvmSet(h, 0, 2, cvmGet(x, 2, 0));
  cvmSet(h, 1, 0, cvmGet(x, 3, 0));
  cvmSet(h, 1, 1, cvmGet(x, 4, 0));
  cvmSet(h, 1, 2, cvmGet(x, 5, 0));
  cvmSet(h, 2, 0, cvmGet(x, 6, 0));
  cvmSet(h, 2, 1, cvmGet(x, 7, 0));
  cvmSet(h, 2, 2, 1);

  float* data = h->data.fl;
  printf("Homography Matrix:\n");
  printf(" %3f, %3f, %3f\n", data[0], data[1], data[2]);
  printf(" %3f, %3f, %3f\n", data[3], data[4], data[5]);
  printf(" %3f, %3f, %3f\n", data[6], data[7], data[8]);
  printf("     %3f should be rightmost, center row\n", cvmGet(h, 1, 2));
  
  return h;
}

//If you find a need for a byte array from your floating point images,
//here's some code you may use.
void convertToByteImage ( IplImage * floatImage, IplImage * byteImage )
{
  assert ( floatImage->depth == IPL_DEPTH_32F );
  assert ( byteImage->depth == IPL_DEPTH_32F );
  cvConvertScale ( floatImage, byteImage, 255, 0 );
}

// Transform point by homography.
void applyHomography ( float   x,
                       float   y,
                       float & xNew,
                       float & yNew,
                       float   h [9] )
{
  float d = h[6] * x + h[7] * y + h[8];
  
  xNew = ( h[0] * x + h[1] * y + h[2] ) / d;
  yNew = ( h[3] * x + h[4] * y + h[5] ) / d;
}

// Compute AUC given a ROC curve
float computeAUC ( vector<ROCPoint> & results )
{
  float auc = 0;
  float xdiff, ydiff;

  for ( size_t i = 1; i < results.size (); ++i )
  {
    xdiff = ( results[i].falseRate - results[i - 1].falseRate );
    ydiff = ( results[i].trueRate - results[i - 1].trueRate );
    auc = auc + xdiff * results[i - 1].trueRate + xdiff * ydiff / 2;
  }
  return auc;
}
