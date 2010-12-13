#include "FaceDetector.h"
#include <stdio.h>

FaceDetector::FaceDetector() 
{
  printf("FaceDetector - Constructor START\n");
  //TODO Put one-time loading stuff here  
  cascade = (CvHaarClassifierCascade*)cvLoad( "haarcascade_frontalface_alt.xml", 0, 0, 0 );
  if(!cascade)
    printf("***ERROR: Failed to load Haar Cascade!\n");
  haarstorage = cvCreateMemStorage(0);
  fisherFace = new Fisher();
}

void FaceDetector::processImage(IplImage* img)
{
  printf("FaceDetector - Process Image\n");
  //TODO Put Processing Code Here.  Replace input image with result.
  
  //Convert incoming image to format useable by our code:
  IplImage* imgbgr = cvCreateImage(cvGetSize(img), IPL_DEPTH_32F, img->nChannels);
  cvCvtColor (img, imgbgr, CV_YCrCb2BGR);
  IplImage* img2 = cvCreateImage(cvGetSize(img), IPL_DEPTH_8U, img->nChannels);
  cvConvertImage(imgbgr, img2); //scale
  
  processImage2(img2); //do stuff
  
  //Convert back to original format
  cvConvertScale(img2, imgbgr, 1.0/255.0);
  cvCvtColor (imgbgr, img, CV_BGR2YCrCb);
  cvReleaseImage(&img2);
  cvReleaseImage(&imgbgr);
}

void FaceDetector::processImage2(IplImage* img)
{
  //Find Faces
  cvClearMemStorage( haarstorage );
  std::vector<Face*> faces = findFaces(img);
  
  //Find label and Mark Faces
  for(uint i = 0; i < faces.size(); i++)
  {
    faces[i]->label = fisherFace->labelFace(faces[i]->image);
    markAndLabel(img, &(faces[i]->location), faces[i]->label);
  }
}

//finds all faces in img, and returns vector of face objects,
//including preprocessed face within each vector
std::vector<Face*> FaceDetector::findFaces(IplImage* img)
{
  std::vector<Face*> faces;
  // Find whether the cascade is loaded, to find the faces. If yes, then:
  if( cascade ) {

    // There can be more than one face in an image. So create a growable sequence of faces.
    // Detect the objects and store them in the sequence
    CvSeq* haarfaces = cvHaarDetectObjects( img,
                                            cascade, 
                                            haarstorage,
                                            1.2, //scale factor (1.1 is more accurate)
                                            2, //min neighbors
                                            CV_HAAR_DO_CANNY_PRUNING,
                                            cvSize(40, 40) //min face size
                                            );

    // Loop the number of faces found.
    for(int i = 0; i < (haarfaces ? haarfaces->total : 0); i++ )
    {
      // Create a new rectangle for drawing the face
      CvRect* r = (CvRect*)cvGetSeqElem( haarfaces, i );
      Face* f = new Face(*r);
      f->image = preprocessFace(img, r);
      faces.push_back(f);
    }
  }
  return faces;
}

//Modifies input image by marking the passed rectange and labeling it with label
void FaceDetector::markAndLabel(IplImage* img, CvRect *square, std::string label)
{
  cvRectangle(img, 
	      cvPoint(square->x, square->y), 
	      cvPoint(square->x + square->width, square->y + square->height),
	      CV_RGB(255,0,0),
	      2);
  CvFont font1, font2;
  double hScale=1.0;
  double vScale=1.0;
  cvInitFont(&font1,CV_FONT_HERSHEY_DUPLEX, hScale,vScale,1.0f, //shear
                                                         1.0f, //thickness
                                                         CV_AA
                                                         );
  cvInitFont(&font2,CV_FONT_HERSHEY_DUPLEX, hScale,vScale,1.0f, //shear
                                                         2.0f, //thickness
                                                         CV_AA
                                                         );
  cvPutText(img,
      	    label.c_str(),
      	    cvPoint(square->x, square->y + square->height),
      	    &font2,
      	    CV_RGB(0,0,0));  
  cvPutText(img,
      	    label.c_str(),
      	    cvPoint(square->x, square->y + square->height),
      	    &font1,
      	    CV_RGB(255,255,255));
}

//Pulls a face (rectangle) out of an image and applies preprocessing.
//input image will not be changed
//a new image is allocated and returned, with size 100x100
IplImage* FaceDetector::preprocessFace(IplImage *img, CvRect *face) {
  cvSetImageROI(img, *face);
  IplImage *faceColor = cvCreateImage(cvSize(100,100), img->depth, img->nChannels);
  cvResize(img, faceColor);
  cvResetImageROI(img);

  IplImage *faceNormalized = cvCreateImage(cvGetSize(faceColor), IPL_DEPTH_8U, 1);

  cvCvtColor(faceColor, faceNormalized, CV_BGR2GRAY);

  cvEqualizeHist(faceNormalized, faceNormalized);

  cvReleaseImage(&faceColor);
  return faceNormalized;
}
