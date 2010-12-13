#include "Face.h"

Face::Face(CvRect rect)
{
  label = "OMGFace!";
  location = rect;
}

Face::~Face()
{
  if(image)
    cvReleaseImage(&image);
}
