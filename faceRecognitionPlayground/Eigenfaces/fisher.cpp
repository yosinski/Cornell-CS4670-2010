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

#define DEBUG
#ifdef DEBUG
#define ImageInfo(x)                                \
    cout << "\n" << __FILE__ << ":" << __LINE__     \
         << " imageInfo(" << #x << ")" << endl;     \
    imageInfo(x);                                   \
    saveImage(__FILE__, __LINE__, #x, x);
#else
#define ImageInfo(x)
#endif

// Forward declarations
void imageInfo (IplImage *);
void saveImage (char *, int, char *, IplImage *);
void matInfo (CvMat *);
void matInfo (cv::Mat);

using namespace std;


// diagonal whitening for covariance matrix
float epsilon = .01;


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
    const char* subjects_filename;
    const char* image_to_classify;
    int nn_dimensions;

    // Check for the correct usage of the command line
    if( argc > 2 ) {
        subjects_filename = argv[1];
        image_to_classify = argv[2];
    } else {
        fprintf( stderr, "Usage: fisher <list_of_subject_files.txt> <image_to_classify.jpg>\n" );
        return -1;
    }

    // Allocate the memory storage
    storage = cvCreateMemStorage(0);
    
    // Create a new named window with title: result
    cvNamedWindow( "result", 1 );



    /******************************
     *  1. Load class_*.txt files and images
     ******************************/

    vector< IplImage* >  images;
    vector< int >        class_ids;
    vector< string >     class_labels;
    vector< string >     image_labels;

    //vector< IplImage* >               temp_class_images;
    //vector< string >                  temp_labels;

    /* assume it is a text file containing the
       list of the single class filenames to be processed - one per line */
    int current_class_id = 1;
    FILE* class_all_file = fopen( subjects_filename, "rt" );
    if( class_all_file ) {
        char buf[1000+1];
        
        // Get the line from the file
        while( fgets( buf, 1000, class_all_file ) ) {

            // Remove the spaces if any, and clean up the name
            int len = (int)strlen(buf);
            while( len > 0 && isspace(buf[len-1]) )
                len--;
            buf[len] = '\0';

            string class_label(buf);

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
                    image_labels.push_back(string(buf));
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
        printf("ERROR: Failure reading class file '%s'!\n", subjects_filename);
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
     *  3. Compute fisher weight vector
     ******************************/


    // 1. Load eigenvectors from files
    vector< IplImage* > eigen_images;

    printf("Loading eigenvectors from eigenvectors.yml... ");
    CvFileStorage* fs2 = cvOpenFileStorage("eigenvectors.yml", NULL, CV_STORAGE_READ);
    char vectorname[50];
    CvFileNode* vectorloc = cvGetFileNodeByName(fs2, NULL, "vector0");
    for(int i = 1; vectorloc != NULL; i++) {
      eigen_images.push_back((IplImage*)cvRead(fs2, vectorloc, &cvAttrList(0,0)));
      //printf("pushed %s\n", vectorname);
      sprintf(vectorname, "vector%d", i);
      vectorloc = cvGetFileNodeByName(fs2, NULL, vectorname);
    }
    //cvReleaseFileStorage(&fs2); This may delete the images
    printf("done.\n%d Eigenvectors (and 1 average) loaded.\n", eigen_images.size()-1);

    if (false) {
        for (int ii = 0; ii < eigen_images.size(); ii++) {
            printf("showing eigen image %d\n", ii);
            //cvConvertScale(eigen_images[ii], eigen_images[ii], 128, 1);
            cvShowImage("result", eigen_images[ii]);
            cout << CV_IMAGE_ELEM(eigen_images[ii], float, 10, 10) << endl;
            cvWaitKey(0);
        }
    }


    int selected_class_id = 1;

    // 2. Project all images onto eigen vectors
    int eigen_dimensions = eigen_images.size() - 1;      // first is average

    IplImage* eigen_array[eigen_dimensions];
    for(int ii = 0; ii < eigen_dimensions; ii++) {
        eigen_array[ii] = eigen_images[ii+1];
    }

    //cv::Mat features(images.size(), eigen_dimensions, CV_32F);

    CvMat* features;
    features = cvCreateMat(images.size(), eigen_dimensions, CV_32F);


    float data[eigen_dimensions];
    for (int ii = 0; ii < images.size(); ii++) {
        cvEigenDecomposite(images[ii],               // image to project
                           eigen_dimensions,         // number of eigen vectors
                           (void*)eigen_array,       // eigen_vectors (starts at 1)
                           0, 0,                     // ioflags, user callback data
                           eigen_images[0],          // average image = first eigen_vector
                           data);                    // output

        for (size_t jj = 0; jj < eigen_dimensions; jj++) {
            //features.at<float>( (int)ii, (int)jj ) = data[jj];
            CV_MAT_ELEM(*features, float, ii, jj) = data[jj];
        }
    }

    //matInfo(features);



    // 3. Sort images into arrays for each class
    vector<int> idx_class0, idx_class1;
    for (int ii = 0; ii < images.size(); ii++) {
        if (class_ids[ii] == selected_class_id)
            idx_class1.push_back(ii);
        else
            idx_class0.push_back(ii);
    }
    int size_class0 = idx_class0.size();
    int size_class1 = idx_class1.size();
    assert(size_class0 > 0 && size_class1 > 0);

    IplImage* images_class0[size_class0];
    IplImage* images_class1[size_class1];

    for(int ii = 0; ii < size_class0; ii++)
        images_class0[ii] = images[idx_class0[ii]];
    for(int ii = 0; ii < size_class1; ii++)
        images_class1[ii] = images[idx_class1[ii]];


    // 4. compute covariance matrices and means for class 0 (not
    // selected) and class 1 (selected)
    CvMat* cov0 = cvCreateMat(eigen_dimensions, eigen_dimensions, CV_32F);
    CvMat* cov1 = cvCreateMat(eigen_dimensions, eigen_dimensions, CV_32F);

    CvMat* mean0 = cvCreateMat(eigen_dimensions, 1, CV_32F);
    CvMat* mean1 = cvCreateMat(eigen_dimensions, 1, CV_32F);

    cvCalcCovarMatrix((const CvArr**)images_class0, size_class0, cov0, mean0, 0);
    cvCalcCovarMatrix((const CvArr**)images_class1, size_class1, cov1, mean1, 0);


    // 5. sum + white noise
    CvMat* covs_whitened = cvCreateMat(eigen_dimensions, eigen_dimensions, CV_32F);
    cvAdd(cov0, cov1, covs_whitened);
    for(int ii = 0; ii < eigen_dimensions; ii++)
        CV_MAT_ELEM(*covs_whitened, float, ii, ii) += epsilon;

    // 6. inverse
    CvMat* covs_inv = cvCreateMat(eigen_dimensions, eigen_dimensions, CV_32F);
    cvInvert(covs_whitened, covs_inv);

    // 7. subtract means (mean01 = mean1 - mean0)
    CvMat* mean01 = cvCreateMat(eigen_dimensions, 1, CV_32F);
    cvAddWeighted(mean0, -1, mean1, 1, 0, mean01);

    // 8. compute w = inv_covs * (u_1 - u_0)
    CvMat* ww = cvCreateMat(eigen_dimensions, 1, CV_32F);
    cvMatMul(covs_inv, mean01, ww);




    /******************************
     *  4. Classify given photo
     ******************************/





    return 0;


}












// Print out image information and one corner of image
void
imageInfo (IplImage * image)
{
    cout << "Image info:" << endl;
    cout << "    nSize:          " << image->nSize          << endl;
    cout << "    ID:             " << image->ID             << endl;
    cout << "    nChannels:      " << image->nChannels      << endl;
    cout << "    alphaChannel:   " << image->alphaChannel   << endl;
    cout << "    depth:          " << image->depth          << endl;
    cout << "    colorModel[4]:  " << image->colorModel[0] << image->colorModel[1]
         << image->colorModel[2] << image->colorModel[3]  << endl;
    cout << "    channelSeq[4]:  " << image->channelSeq[0] << image->channelSeq[1]
         << image->channelSeq[2] << image->channelSeq[3] << endl;
    cout << "    dataOrder:      " << image->dataOrder      << endl;
    cout << "    origin:         " << image->origin         << endl;
    cout << "    align:          " << image->align          << endl;
    cout << "    width:          " << image->width          << endl;
    cout << "    height:         " << image->height         << endl;
    cout << "    imageSize:      " << image->imageSize      << endl;
    cout << "    widthStep:      " << image->widthStep      << endl;
    cout << "    BorderMode[4]:  " << image->BorderMode[4]  << endl;
    cout << "    BorderConst[4]: " << image->BorderConst[4] << endl;

    int ii, jj, cc;
    int nChannels = image->nChannels;
    
    int maxIdx = 8;

    for (ii = 0; ii < min(maxIdx,image->height); ++ii) {
        for (jj = 0; jj < min(maxIdx,image->width); ++jj) {
            cout << ii << "," << jj << " " << "(";
            for (cc = 0; cc < nChannels; ++cc) {
                printf("%.2f", CV_IMAGE_ELEM(image, float, ii, jj*nChannels+cc));
            }
            cout << ") ";
        }
        cout << endl;
    }
}

void saveImage(char * file, int line, char * name, IplImage * image)
{
    char buffer [50];
    //sprintf (buffer, "%s_%03d_%s.png", file, line, name);
    sprintf (buffer, "line_%03d_%s.png", line, name);

    IplImage* temp = cvCloneImage(image);
    cvConvertScale(image, temp, 255, 0);
    cvSaveImage(buffer, temp);
    cvReleaseImage(&temp);
}

// Print out matrix information and matrix itself
void
matInfo (cv::Mat mat)
{
    matInfo((CvMat *) &mat);
}

// Print out matrix information and matrix itself
void
matInfo (CvMat * mat)
{
    cout << "Matrix info:" << endl;
    cout << "    rows:           " << mat->rows             << endl;
    cout << "    cols:           " << mat->cols             << endl;

    int ii, jj;

    for (ii = 0; ii < mat->rows; ++ii) {
        cout <<  "[ ";
        for (jj = 0; jj < mat->cols; ++jj) {
            cout << CV_MAT_ELEM(*mat, float, ii, jj) << "   ";
        }
        cout <<  " ]" << endl;
    }
}
