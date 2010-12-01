// OpenCV Sample Application: facedetect.c

// Include header files
#include "opencv/cv.h"
#include "opencv/highgui.h"
#include "opencv/cvaux.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <float.h>
#include <limits.h>
#include <time.h>
#include <ctype.h>
#include <vector>

// Create memory for calculations
static CvMemStorage* storage = 0;

// Main function, defines the entry point for the program.
int main( int argc, char** argv )
{

    // Structure for getting video from camera or avi
    CvCapture* capture = 0;

    // Images to capture the frame from video or camera or from file
    IplImage *frame, *frame_copy = 0;

    // Input file name for avi or image file.
    const char* vectorList;
    const char* testImage;

    // Check for the correct usage of the command line
    if( argc > 2 )
    {
        vectorList = argv[1];
	testImage = argv[2];
    }
    else
    {
        fprintf( stderr, "Usage: eigenDecomp eigenVectorList testImage\n" );
        return -1;
    }

    // Allocate the memory storage
    storage = cvCreateMemStorage(0);
    
    // Create a new named window with title: result
    cvNamedWindow( "result", 1 );

    std::vector<IplImage*> images;

    /* assume it is a text file containing the
       list of the image filenames to be processed - one per line */
    FILE* f = fopen( vectorList, "rt" );
    if( f )
    {
        char buf[1000+1];

	// Get the line from the file
	while( fgets( buf, 1000, f ) )
	{

	    // Remove the spaces if any, and clean up the name
	    int len = (int)strlen(buf);
	    while( len > 0 && isspace(buf[len-1]) )
	        len--;
	    buf[len] = '\0';

	    // Load the image from the filename present in the buffer
	    IplImage* image = cvLoadImage( buf, CV_LOAD_IMAGE_GRAYSCALE );

	    // If the image was loaded succesfully, then:
	    if( image )
	    {
	      // Add image to vector
	      IplImage* converted = cvCreateImage(cvGetSize(image), IPL_DEPTH_32F, 1);
	      cvConvertScale(image, converted, 1/255.0);
	      images.push_back(converted);
	      // Wait for the user input, and release the memory
	      //cvShowImage("result", converted);
	      //cvWaitKey(0);
	      cvReleaseImage( &image );
	    }
	}
	// Close the file
	fclose(f);
    }

    //TODO: unfix this number!
    printf("FLANN DIMS: %d, %d\n", 165, images.size());
    cv::Mat mat( 165, images.size(), CV_32F );
    cv::flann::Index::Index flannIndex(mat, cv::flann::SavedIndexParams("flann.dat"));
    
    IplImage* eigenArray[images.size()-1];
    
    IplImage* avgImage = images[0]; 
    for(int i = 1; i < images.size(); i++) {
      eigenArray[i-1] = images[i];
      //      cvShowImage("result", imageArray[i]);
      //      cvWaitKey(0);
    }

    //load test image
    IplImage* testImg = cvLoadImage(testImage, CV_LOAD_IMAGE_GRAYSCALE);
    float projection[images.size()];

    // Project the test image onto the PCA subspace
    cvEigenDecomposite(
		       testImg, //test object
		       images.size()-1, //number of eigen vectors
		       (void*)eigenArray, //eigenVectors
		       0, 0, //ioflags, user callback data
		       avgImage, //root eigen vector
		       projection);

    
    //print projection
    printf("Eigenvector Coefficents:\n");
    for(int i = 0; i < images.size() - 1; i++) //-1 since one less eigenvalue than images
      printf("%5f ", projection[i]);
    printf("\n");

    std::vector<float> proj(images.size());
    std::vector<float> dists(1);
    std::vector<int> indicies(1);
    for(int i = 0; i < images.size(); i++)
      proj[i] = projection[i];
    
    flannIndex.knnSearch(proj, indicies, dists, 1, NULL);
    printf("Index Match: %d, dist: %f\n", indicies[0], dists[0]);
			
    // Destroy the window previously created with filename: "result"
    cvDestroyWindow("result");
    printf("Done.\n");
    // return 0 to indicate successfull execution of the program
    return 0;
}
