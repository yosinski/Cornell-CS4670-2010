#ifndef FACEDETECTOR_H
#define FACEDETECTOR_H

#include <vector>
#include "opencv/highgui.h"
#include "opencv/cv.h"
#include "Face.h"
#include "Fisher.h"

class FaceDetector
{
  public:
    FaceDetector();
    ~FaceDetector() {}
    void processImage(IplImage* img);
    void processImage2(IplImage* img);
    
  private:
    Fisher* fisherFace;
    CvHaarClassifierCascade* cascade;  
    CvMemStorage* haarstorage;
    std::vector<Face*> findFaces(IplImage* img);
    void markAndLabel(IplImage* img, CvRect *square, std::string label);
    IplImage* preprocessFace(IplImage *img, CvRect *face);
};

#endif
