// OpenCV Sample Application: facedetect.c

// Include header files
#include "opencv/cv.h"
#include "opencv/highgui.h"
#include "opencv/cvaux.h"

#include <iostream>
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

int min(int a, int b) { return a > b ? b : a; }

// Main function, defines the entry point for the program.
int main( int argc, char** argv )
{

    // Structure for getting video from camera or avi
    CvCapture* capture = 0;

    // Images to capture the frame from video or camera or from file
    IplImage *frame, *frame_copy = 0;

    // Input file name for avi or image file.
    const char* input_name;
    int nn_dimensions;

    // Check for the correct usage of the command line
    if( argc > 1 ) {
        input_name = argv[1];
    } else {
        fprintf( stderr, "Usage: fisher <list_of_subject_files.txt>\n" );
        return -1;
    }

    // Allocate the memory storage
    storage = cvCreateMemStorage(0);
    
    // Create a new named window with title: result
    cvNamedWindow( "result", 1 );




    /******************************
     *  1. Load class_*.txt files and images
     ******************************/

    std::vector< IplImage* >  images;
    std::vector< int >        class_ids;
    std::vector< std::string >     class_labels;
    std::vector< std::string >     image_labels;

    //std::vector< IplImage* >               temp_class_images;
    //std::vector< string >                  temp_labels;

    /* assume it is a text file containing the
       list of the single class filenames to be processed - one per line */
    int current_class_id = 1;
    FILE* class_all_file = fopen( input_name, "rt" );
    if( class_all_file ) {
        char buf[1000+1];
        
        // Get the line from the file
        while( fgets( buf, 1000, class_all_file ) ) {

            // Remove the spaces if any, and clean up the name
            int len = (int)strlen(buf);
            while( len > 0 && isspace(buf[len-1]) )
                len--;
            buf[len] = '\0';

            std::string class_label(buf);

            //temp_class_images.clear();
            //temp_labels.clear();

            FILE* class_subject_file = fopen( buf, "rt" );

            if (class_subject_file) {
                // Get the line from the file
                while( fgets( buf, 1000, class_subject_file ) ) {
                    // Remove the spaces if any, and clean up the name
                    int len = (int)strlen(buf);
                    while( len > 0 && isspace(buf[len-1]) )
                        len--;
                    buf[len] = '\0';

                    // Load the image from the filename present in the buffer
                    IplImage* image = cvLoadImage( buf, CV_LOAD_IMAGE_GRAYSCALE );

                    // If the image was loaded succesfully, then:
                    if( ! image ) {
                        printf("ERROR: Failed to load image: %s\n", buf);
                    }

                    // 1. save class label (e.g. class_subject01.txt)
                    class_labels.push_back(class_label);
                    // 2. save class id (e.g. 1)
                    class_ids.push_back(current_class_id);
                    // 3. save image label
                    image_labels.push_back(std::string(buf));
                    // 4. save image
                    images.push_back(image);
                }
            } else {
                printf("ERROR: Failed to read subject file '%s'!\n", buf);
            }

            fclose(class_subject_file);
        
            current_class_id++;
        }
        // Close the file
        fclose(class_all_file);
    } else {
        printf("ERROR: Failure reading class file '%s'!\n", input_name);
    }



    /******************************
     *  2. Show images loaded
     ******************************/

    if (false) {
        for (int ii = 0; ii < images.size(); ii++) {
            printf("showing image %d\n", ii);
            cvShowImage("result", images[ii]);
            cvWaitKey(0);
        }
    }



    /******************************
     *  3. Fisher awesomeness...
     ******************************/

    int selected_class_id = 1;

    // 1. compute cov class 0 (not selected)
    // 2. compute cov class 1 (selected)
    // 3. sum + white noise
    // 4. inverse
    // 5. compute mean of class 0 (not selected)
    // 6. compute mean of class 1 (selected)
    // 7. compute w = inv_covs * (u_1 - u_0)

    return 0;


    IplImage* imageArray[images.size()];
    IplImage* eigenArray[images.size()-1];
    
    for(int i = 0; i < images.size(); i++) {
        imageArray[i] = images[i];
        if(i < images.size()-1)
            eigenArray[i] = cvCreateImage(cvGetSize(images[i]), IPL_DEPTH_32F, 1);
        //      cvShowImage("result", imageArray[i]);
        //      cvWaitKey(0);
    }

    //cvTermCriteria: 
    CvTermCriteria calcLimit = cvTermCriteria(CV_TERMCRIT_ITER, images.size()-1, 0);
    
    float lambdas[images.size() - 1];
    IplImage* avgImage = cvCreateImage(cvGetSize(images[0]), IPL_DEPTH_32F, 1);
    
    //Now that all of the images are loaded, begin Eigenfaces/PCA:
    cvCalcEigenObjects(images.size(),  //number of source objects
                       (void*)imageArray, //array of source objects (IplImage*)
                       (void*)eigenArray, //array of destination objects
                       CV_EIGOBJ_NO_CALLBACK, //ioflags
                       0, //io buffer size
                       0, //user data
                       &calcLimit, //calculation limit
                       avgImage, //average image
                       lambdas //eigenvalues (return)
                       );

    //print eigenvalues
    printf("Lambdas:\n");
    for(int i = 0; i < images.size() - 1; i++) //-1 since one less eigenvalue than images
        printf("%5f ", lambdas[i]);
    printf("\n");

    int projection_dims = 0;
    while(!cvIsNaN(lambdas[projection_dims]) && projection_dims < images.size())
        projection_dims++;

    //populate flann
    printf("FLANN DIMS: %d, %d\n", images.size(), min(projection_dims, nn_dimensions));
    cv::Mat features ( images.size (), min(projection_dims, nn_dimensions), CV_32F );
    
    for ( size_t i = 0; i < images.size (); ++i ) {
        float data[projection_dims];
        cvEigenDecomposite(images[i], //test object
                           projection_dims, //number of eigen vectors
                           (void*)eigenArray, //eigenVectors
                           0, 0, //ioflags, user callback data
                           avgImage, //root eigen vector
                           data);
        for ( size_t j = 0; j < min(projection_dims, nn_dimensions); ++j ) {
            features.at<float>( ( int )i, ( int )j ) = data[j];
        }
        //printf("Eigenvector Coefficents:\n");
        //for(int j = 0; j < projection_dims; j++) //-1 since one less eigenvalue than images
        //printf("%5f ", data[j]);
        //printf("\n");
    }

    //Save neighbor data to disk
    CvFileStorage* fs = cvOpenFileStorage("nn.yml", NULL, CV_STORAGE_WRITE);
    cvWrite(fs, "data", &((CvMat)features), cvAttrList(0,0));
    cvReleaseFileStorage(&fs);
    
    //Save eigenvectors to disk
    CvFileStorage* fs2 = cvOpenFileStorage("eigenvectors.yml", NULL, CV_STORAGE_WRITE);
    cvWrite(fs2, "vector0", avgImage, cvAttrList(0,0));
    char filename[50];
    for(int i = 0; i < projection_dims; i++) {
        sprintf(filename, "vector%d", i+1);
        cvWrite(fs2, filename, eigenArray[i], cvAttrList(0,0));
    }
    cvReleaseFileStorage(&fs2);

    //save labels to disk
    FILE* lf = fopen( "labels.txt", "w" );
    for(int i = 0; i < class_labels.size(); i++) 
      fprintf(lf, "%s\n", class_labels[i]);
    fclose(lf);

    //cv::flann::Index::Index flannIndex ( features, cv::flann::KDTreeIndexParams () );
    //flannIndex.save("flann.dat");
    

    /*
    IplImage* uimg = cvCreateImage(cvGetSize(images[0]), IPL_DEPTH_8U, 1);

    //show eigenfaces
    for(int i = 0; i < images.size(); i++) {
      if(cvIsNaN(lambdas[i]))
	break;

      IplImage* scaleImg;
      
      if(i == 0)
	scaleImg = avgImage;
      else
	scaleImg = eigenArray[i-1];

      double minVal, maxVal;
      cvMinMaxLoc(scaleImg, &minVal, &maxVal);
      // Deal with NaN and extreme values, since the DFT seems to give some NaN results.
      if (cvIsNaN(minVal) || minVal < -1e30)
	minVal = -1e30;
      if (cvIsNaN(maxVal) || maxVal > 1e30)
	maxVal = 1e30;
      if (maxVal-minVal == 0.0f)
	maxVal = minVal + 0.001;// remove potential divide by zero errors.
      cvConvertScale(scaleImg, uimg, 255.0/(maxVal-minVal), -minVal*255.0/(maxVal-minVal));

      char filename[100];
      sprintf(filename, "eigenvector%05d.bmp", i);
      cvSaveImage(filename, uimg);

      cvShowImage("result", uimg);
      cvWaitKey(20);
    }
    */
			
    // Destroy the window previously created with filename: "result"
    cvDestroyWindow("result");
    printf("Done.\n");
    // return 0 to indicate successfull execution of the program
    return 0;
}
