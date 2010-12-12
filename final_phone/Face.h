#ifndef FACE_H
#define FACE_H

#include "opencv/highgui.h"

class Face
{
  public:
    Face(CvRect rect);
    ~Face();
    CvRect location;
    IplImage* image;
    
};

#endif
      
