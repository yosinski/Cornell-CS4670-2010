#ifndef FEATURES_H
#define FEATURES_H

#include "ImageDatabase.h"
#include <opencv/cv.h>
#ifdef Q_WS_MAEMO_5
 #include "Progressable.h"
#endif

struct ROCPoint
{
  float trueRate;
  float falseRate;
};

IplImage * constructPanorama ( IplImage *, IplImage *, int, int
#ifdef Q_WS_MAEMO_5
                               , Progressable * = 0
#endif
                               );

void selectInliers ( const std::vector<Feature> &,
                     const std::vector<Feature> &,
                     std::      vector<FeatureMatch> &,
                     float );

IplImage * compositeImages ( IplImage *, IplImage *, CvMat *
#ifdef Q_WS_MAEMO_5
                             , Progressable * = 0
#endif
                             );

void applyHomography ( CvMat *, std::vector<Feature> & );

CvMat * computeHomography ( const std:: vector<Feature> &,
                            const std:: vector<Feature> &,
                            const std:: vector<FeatureMatch> &,
                            CvMat *     h );

CvMat * ransacHomography ( const std::vector<Feature> &,
                           const std::vector<Feature> &,
                           const std::vector<FeatureMatch> &,
                           int,
                           int
#ifdef                                Q_WS_MAEMO_5
                           ,
                           Progressable * = 0
#endif
                           );

// Compute harris values of an image.
void computeMOPSValues ( IplImage * srcImage,
                         IplImage * destImage,
                         IplImage * orientationImage );

//  Compute local maximum of MOPS values in an image.
void computeLocalMaxima ( IplImage * srcImage, IplImage * destImage );

// Compute features of an image.
bool computeFeatures ( IplImage * image,
                       FeatureSet & features,
                       int featureType
#ifdef Q_WS_MAEMO_5
                       , Progressable * = 0
#endif
                       );

// Perform a query on the database.
bool performQuery ( const FeatureSet &f1,
                    const ImageDatabase &db,
                    size_t & bestIndex,
                    vector<FeatureMatch> &bestMatches,
                    float &bestScore,
                    int matchType
#ifdef Q_WS_MAEMO_5
                    , Progressable * = 0
#endif
                    );

// Match one feature set with another.
bool matchFeatures ( const FeatureSet &f,
                     const FeatureSet &f2,
                     vector<FeatureMatch> &matches,
                     float &totalScore,
                     int matchType
#ifdef Q_WS_MAEMO_5
                     , Progressable * = 0
#endif
                     );

// Add ROC curve data to the data vector
void addRocData ( const FeatureSet &f1,
                  const FeatureSet &f2,
                  const vector<FeatureMatch> &matches,
                  float h[9],
                  vector<bool> &isMatch,
                  float threshold,
                  float &maxD );

// Evaluate a match using a ground truth homography.
float evaluateMatch ( const FeatureSet &f1,
                      const FeatureSet &f2,
                      const vector<FeatureMatch> &matches,
                      float h[9] );

// Compute silly example features.
void dummyComputeFeatures ( IplImage * image, FeatureSet & features
#ifdef Q_WS_MAEMO_5
                            , Progressable * = 0
#endif
                            );

// Compute actual feature
void ComputeMOPSFeatures ( IplImage * image, FeatureSet & features
#ifdef Q_WS_MAEMO_5
                           , Progressable * = 0
#endif
                           );

void ComputeSURFFeatures ( IplImage * image, FeatureSet & features
#ifdef Q_WS_MAEMO_5
                           , Progressable * = 0
#endif
                           );


// Perform ssd feature matching.
void ssdMatchFeatures ( const FeatureSet &f1,
                        const FeatureSet &f2,
                        vector<FeatureMatch> &matches,
                        float &totalScore
#ifdef Q_WS_MAEMO_5
                        , Progressable * = 0
#endif
                        );

// Perform ratio feature matching.  You must implement this.
void ratioMatchFeatures ( const FeatureSet &f1,
                          const FeatureSet &f2,
                          vector<FeatureMatch> &matches,
                          float &totalScore
#ifdef Q_WS_MAEMO_5
                          , Progressable * = 0
#endif
                          );
void convertToByteImage ( IplImage * floatImage, IplImage * byteImage );

// Compute SSD distance between two vectors.
float distanceSSD ( const vector<float> & v1, const vector<float> & v2 );

// Transform point by homography.
void applyHomography ( float x,
                       float y,
                       float &xNew,
                       float &yNew,
                       float h[9] );

// Computes points on the Roc curve
vector<ROCPoint> computeRocCurve ( vector<FeatureMatch> & matches,
                                   vector<bool> &         isMatch,
                                   vector<float> &        thresholds );

// Compute AUC given a ROC curve
float computeAUC ( vector<ROCPoint> & results );

#endif
