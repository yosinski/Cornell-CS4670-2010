#ifndef FISHER_H
#define FISHER_H

#include "opencv/highgui.h"
#include <vector>
#include <string.h>

class Fisher
{
  public:
    Fisher();
    ~Fisher() {}
    std::string labelFace(IplImage* face);
    
  private:
    IplImage* avgImage;
    IplImage** eigen_array;
    int eigen_dimensions;
    int num_classes;
    std::vector< CvMat* > weights;
    std::vector< float >  scores_mean0;
    std::vector< float >  scores_mean1;
    std::vector< std::string >     class_labels;
    void reshapeAndSave(const char* filename, const CvMat* mat, CvSize destination_size);
    void autoScaleImage (IplImage* in, IplImage* out, float min_goal, float max_goal);
    void matToImg(const CvMat * in, IplImage * out);

};

#endif
