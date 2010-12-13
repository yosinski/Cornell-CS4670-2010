#ifndef FACE_H
#define FACE_H

#include "opencv/highgui.h"
#include <string.h>

class Face
{
  public:
    Face(CvRect rect);
    ~Face();
    CvRect location;
    IplImage* image;
    std::string label;
};

#endif
      
