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
    IplImage* inputimg = cvLoadImage(argv[i], CV_LOAD_IMAGE_COLOR);
    if(inputimg)
    {
      //IplImage* YUVimg = cvCreateImage(cvGetSize(inputimg), IPL_DEPTH_8U, inputimg->nChannels);
      //cvConvertImage(inputimg, YUVimg, CV_BGR2YCrCb); //phone use YUV
      //IplImage* fimg = cvCreateImage(cvGetSize(inputimg), IPL_DEPTH_32F, inputimg->nChannels);
      //cvConvertScale(YUVimg, fimg, 1.0/255.0); //phone use YUV
      //fd->processImage(fimg);
      //cvConvertScale(fimg, YUVimg, 255);
      //cvConvertImage(YUVimg, inputimg, CV_YCrCb2BGR); //phone use YUV
      //cvShowImage("result", inputimg);
      
      fd->processImage2(inputimg);
      cvShowImage("result", inputimg);
      
      cvWaitKey(0);
      //cvReleaseImage(&YUVimg);
      cvReleaseImage(&inputimg);
      //cvReleaseImage(&fimg);
    }else
      printf("Could Not Open Image: %s\n", argv[i]);
  }
  cvDestroyWindow("result");
  return 0;
}
