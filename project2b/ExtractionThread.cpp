#include "ExtractionThread.h"
#include "Features.h"

ExtractionThread::ExtractionThread ( QWidget *   parent,
                                     FeatureType featureType,
                                     IplImage *  img ) : QThread ( parent ),
  featureType ( featureType ), img ( img )
{
}

void ExtractionThread::run ()
{
  assert ( img->depth == IPL_DEPTH_8U );
  assert ( img->nChannels == 3 );

  IplImage * image = cvCreateImage ( cvGetSize ( img ), IPL_DEPTH_32F, 3 );
  cvConvertScale ( img, image, 1 / 255., 0 );

  computeFeatures ( image, features, featureType, this );

  cvReleaseImage ( &image );
}

const FeatureSet & ExtractionThread::getFeatures ()
{
  return features;
}
