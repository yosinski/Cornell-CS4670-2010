#include "FaceDetector.h"
#include <stdio.h>

int main( int argc, char** argv )
{
  // Check for the correct usage of the command line
  if( argc < 2 )
  {
    fprintf( stderr, "Usage: maintest image1 [image2] [image3]...\n" );
    return -1;
  }
  FaceDetector* fd = new FaceDetector();
  cvNamedWindow( "result", 1 );
  for(int i = 1; i < argc; i++)
  {
    IplImage* inputimg = cvLoadImage(argv[i]);
    if(inputimg)
    {
      IplImage* YUVimg = cvCloneImage(inputimg);
      cvConvertImage(inputimg, YUVimg, CV_BGR2YCrCb); //phone use YUV
      fd->processImage(YUVimg);
      cvShowImage("result", YUVimg);
      cvWaitKey(0);
      cvReleaseImage(&YUVimg);
      cvReleaseImage(&inputimg);
    }else
      printf("Could Not Open Image: %s\n", argv[i]);
  }
  cvDestroyWindow("result");
  return 0;
}
