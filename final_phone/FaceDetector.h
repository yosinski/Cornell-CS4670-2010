#ifndef FACEDETECTOR_H
#define FACEDETECTOR_H

#include <vector>
#include "opencv/highgui.h"
#include "opencv/cv.h"
#include "Face.h"

class FaceDetector
{
  public:
    FaceDetector();
    ~FaceDetector() {}
    void processImage(IplImage* img);
    
  private:
    CvHaarClassifierCascade* cascade;  
    CvMemStorage* haarstorage;
    char* findLabel(IplImage* face);
    std::vector<Face*> findFaces(IplImage* img);
    void markAndLabel(IplImage* img, CvRect *square, char* label);
    IplImage* preprocessFace(IplImage *img, CvRect *face);
};

#endif
