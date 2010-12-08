// OpenCV Sample Application: facedetect.c

// Include header files
#include "opencv/cv.h"
#include "opencv/highgui.h"
#include "opencv/cvaux.h"
//#include "flann/flann.h"
//#include "flann/io/hdf5.h"
// To use our own flann, the flann library from opencv
// must be removed.  to do this, comment out the include in cxcore.hpp:
// #include "cxflann.h"
// I know this isnt a good way of doing this, but it works and is easy.

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
    if( argc == 2 )
    {
      //vectorList = argv[1];
      testImage = argv[1];
    }
    else
    {
      fprintf( stderr, "Usage: eigenDecomp testImage\n" );
      return -1;
    }

    // Allocate the memory storage
    storage = cvCreateMemStorage(0);
    
    // Create a new named window with title: result
    cvNamedWindow( "result", 1 );

    std::vector<IplImage*> images;
    std::vector<char*> labels;

    //Load Labels
    printf("Loading Labels...  ");
    FILE* f = fopen( "labels.txt", "rt" );
    if( f )
    {
        char buf[100+1];

	// Get the line from the file
	while( fgets( buf, 100, f ) )
	{

	    // Remove the spaces if any, and clean up the name
	    int len = (int)strlen(buf);
	    while( len > 0 && isspace(buf[len-1]) )
	        len--;
	    buf[len] = '\0';

	    char* str = (char*)malloc(sizeof(char)*100);
	    memcpy(str, buf, 100);
	    labels.push_back(str);
	}
	// Close the file
	fclose(f);
	printf("%d Labels loaded.\n", labels.size());
    }else
      printf("Failed.\n");

    //Load Eigenvectors:
    printf("Loading Eigenvectors...  ");
    CvFileStorage* fs2 = cvOpenFileStorage("eigenvectors.yml", NULL, CV_STORAGE_READ);
    char vectorname[50];
    CvFileNode* vectorloc = cvGetFileNodeByName(fs2, NULL, "vector0");
    for(int i = 1; vectorloc != NULL; i++) {
      images.push_back((IplImage*)cvRead(fs2, vectorloc, &cvAttrList(0,0)));
      //printf("pushed %s\n", vectorname);
      sprintf(vectorname, "vector%d", i);
      vectorloc = cvGetFileNodeByName(fs2, NULL, vectorname);
    }
    //cvReleaseFileStorage(&fs2); This may delete the images
    printf("%d Eigenvectors (and 1 average) loaded.\n", images.size()-1);

    //TODO: unfix this number! - Done!
    //printf("FLANN DIMS: %d, %d\n", 165, images.size());
    //cv::Mat mat( 165, images.size(), CV_32F );
    //flann::Matrix<float> flannmat;
    //flann::load_from_file(flannmat, "flann.dat", "flann.dat"); 
    //flann::Index::Index flannIndex(flannmat, flann::LinearIndexParams());
    printf("Loading Nearest Neighbor Matrix...  ");
    CvMat* flannmat;
    CvFileStorage* fs = cvOpenFileStorage("nn.yml", NULL, CV_STORAGE_READ);
    flannmat = (CvMat*)cvRead(fs, cvGetFileNodeByName(fs, NULL, "data"), &cvAttrList(0,0));
    //cvReleaseFileStorage(&fs); This will delete the matrix
    printf("Done. DIMS: %d, %d\n", flannmat->rows, flannmat->cols);
    cv::flann::Index::Index flannIndex(flannmat, cv::flann::LinearIndexParams());

    int nn_dims = flannmat->cols; //number of nearest neighbor dimensions available
    int projection_dims = images.size()-1; //number of dimensions to project into eigenspace.

    IplImage* eigenArray[projection_dims];
    
    IplImage* avgImage = images[0]; 
    for(int i = 0; i < projection_dims; i++) {
      eigenArray[i] = images[i+1];
    }

    //load test image
    printf("Loading Test Image...\n");
    IplImage* testImg = cvLoadImage(testImage, CV_LOAD_IMAGE_GRAYSCALE);
    float projection[projection_dims];

    // Project the test image onto the PCA subspace
    printf("Conducting Eigen Decomposite...\n");
    cvEigenDecomposite(
		       testImg, //test object
		       projection_dims, //number of eigen vectors
		       (void*)eigenArray, //eigenVectors
		       0, 0, //ioflags, user callback data
		       avgImage, //root eigen vector
		       projection);

    
    //print projection
    //printf("Eigenvector Coefficents:\n");
    //for(int i = 0; i < projection_dims; i++) 
	//printf("%5f ", projection[i]);
    //printf("\n");

    int neighbors = 10; //to test against

    std::vector<float> proj(nn_dims);
    std::vector<float> dists(neighbors);
    std::vector<int> indicies(neighbors);
    for(int i = 0; i < nn_dims; i++)
      proj[i] = projection[i];

    flannIndex.knnSearch(proj, indicies, dists, neighbors, NULL);
    for(int i = 0; i < neighbors; i++)
      printf("Index Match0: %4d, dist: %13.3f, Label: %s\n", 
	     indicies[i], dists[i], labels[indicies[i]]);

    // Destroy the window previously created with filename: "result"
    cvDestroyWindow("result");
    printf("Done.\n");
    // return 0 to indicate successfull execution of the program
    return 0;
}
