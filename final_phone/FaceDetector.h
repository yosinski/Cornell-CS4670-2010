#ifndef FACEDETECTOR_H
#define FACEDETECTOR_H

#include "opencv/highgui.h"

class FaceDetector
{
  public:
    FaceDetector();
    ~FaceDetector();
    void processImage(IplImage* img);
};

#endif
