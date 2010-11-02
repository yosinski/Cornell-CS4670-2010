#include "PanoramaThread.h"
#include "FeatureSet.h"
#include "Features.h"
#include <opencv/highgui.h>
#include "Panorama.h"

PanoramaThread::PanoramaThread ( Panorama *  parent,
                                 IplImage *  img1,
                                 IplImage *  img2,
                                 FeatureType featureType,
                                 MatchType   matchType )
  : QThread ( parent ),
  panorama ( parent ), img1 ( img1 ),
  img2 ( img2 ), featureType ( featureType ), matchType ( matchType )
{
}

void PanoramaThread::run ()
{
  IplImage * img1_float = cvCreateImage ( cvGetSize ( img1 ), IPL_DEPTH_32F, 3 );

  cvConvertScale ( img1, img1_float, 1 / 255., 0 );
  IplImage * img2_float = cvCreateImage ( cvGetSize ( img2 ), IPL_DEPTH_32F, 3 );
  cvConvertScale ( img2, img2_float, 1 / 255., 0 );
  output = constructPanorama ( img1_float,
                               img2_float,
                               featureType,
                               matchType,
                               this );
  cvReleaseImage ( &img1_float );
  cvReleaseImage ( &img2_float );
}

IplImage * PanoramaThread::getComposite ()
{
  return output;
}
