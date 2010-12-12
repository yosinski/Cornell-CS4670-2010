#include "Face.h"

Face::Face(CvRect rect)
{
  location = rect;
}

Face::~Face()
{
  if(image)
    cvReleaseImage(&image);
}
