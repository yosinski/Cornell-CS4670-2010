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
#define MatInfo(x)                                  \
    cout << "\n" << __FILE__ << ":" << __LINE__     \
         << " matInfo(" << #x << ")" << endl;       \
    matInfo(x);
#else
#define ImageInfo(x)
#define MatInfo(x)
#endif

// Forward declarations
void imageInfo (IplImage *);
void saveImage (char *, int, char *, IplImage *);
void matInfo (CvMat *);
void matInfo (cv::Mat);

using namespace std;


// diagonal whitening for covariance matrix
float epsilon = 1.0;
int default_max_eigen_dimensions = 60;
int max_eigen_dimensions = default_max_eigen_dimensions;

// Create memory for calculations
static CvMemStorage* storage = 0;

int min(int a, int b) { return a > b ? b : a; }

// Main function, defines the entry point for the program.
int main( int argc, char** argv )
{
    // Input file name for avi or image file.
    const char* subjects_filename;
    const char* image_to_classify;

    // Process command line arguments
    if( argc > 3 ) {
        max_eigen_dimensions = atoi(argv[3]);
    }

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

    vector< string >     class_labels;       // short list of class labels
    vector< IplImage* >  images;             // long list of images
    vector< int >        class_ids;          // long list of class ids
    vector< string >     image_labels;       // long list of image labels

    //vector< IplImage* >               temp_class_images;
    //vector< string >                  temp_labels;

    /* assume it is a text file containing the
       list of the single class filenames to be processed - one per line */
    int num_classes = 0;  // one more than the highest numbered class
    FILE* class_all_file = fopen( subjects_filename, "rt" );
    if( class_all_file ) {
        char buf[1000+1];
        char buf2[1000+1];
        
        // Get the line from the file
        while( fgets( buf, 1000, class_all_file ) ) {

            // Remove the spaces if any, and clean up the name
            int len = (int)strlen(buf);
            while( len > 0 && isspace(buf[len-1]) )
                len--;
            buf[len] = '\0';

            string class_label(buf);
            class_labels.push_back(class_label);

            //temp_class_images.clear();
            //temp_labels.clear();

            FILE* class_subject_file = fopen( buf, "rt" );

            if (class_subject_file) {
                // Get the line from the file
                while( fgets( buf2, 1000, class_subject_file ) ) {
                    // Remove the spaces if any, and clean up the name
                    int len = (int)strlen(buf2);
                    while( len > 0 && isspace(buf2[len-1]) )
                        len--;
                    buf2[len] = '\0';

                    // Load the image from the filename present in the buffer
                    IplImage* image = cvLoadImage( buf2, CV_LOAD_IMAGE_GRAYSCALE );

                    // If the image was loaded succesfully, then:
                    if( ! image ) {
                        printf("ERROR: Failed to load image: %s\n", buf2);
                    }

                    // 1. save class label (e.g. class_subject01.txt)
                    //class_labels.push_back(class_label);
                    // 2. save class id (e.g. 1)
                    class_ids.push_back(num_classes);
                    // 3. save image label
                    string temp(buf2);
                    //printf("Read %s\n", temp.c_str());
                    image_labels.push_back(temp);
                    // 4. save image
                    images.push_back(image);
                }
            } else {
                printf("ERROR: Failed to read subject file '%s'!\n", buf);
            }

            fclose(class_subject_file);
        
            num_classes++;
        }
        // Close the file
        fclose(class_all_file);
    } else {
        printf("ERROR: Failure reading class file '%s'!\n", subjects_filename);
    }

    // vector< int > class_ids_unique = class_ids;
    // sort(class_ids_unique.begin(), class_ids_unique.end());
    // unique(class_ids_unique.begin(), class_ids_unique.end());



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
        IplImage* temp = (IplImage*)cvRead(fs2, vectorloc, &cvAttrList(0,0));
        eigen_images.push_back(temp);
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


    // 2. Project all images onto eigen vectors
    printf("Projecting all images onto eigenvectors\n");
    int eigen_dimensions = eigen_images.size() - 1;      // first is average

    // [JBY] use this to control how many dimensions we use
    eigen_dimensions = min(max_eigen_dimensions, eigen_dimensions);

    printf("Using first %d eigen vectors\n", eigen_dimensions);

    IplImage* eigen_array[eigen_dimensions];
    for(int ii = 0; ii < eigen_dimensions; ii++) {
        eigen_array[ii] = eigen_images[ii+1];
    }

    //cv::Mat features(images.size(), eigen_dimensions, CV_32F);

    CvMat* features = cvCreateMat(images.size(), eigen_dimensions, CV_32F);

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

    //MatInfo(features);



    vector< CvMat* > weights;
    vector< float >  scores_mean0;
    vector< float >  scores_mean1;

    // For each subject
    for (int cc = 0; cc < num_classes; cc++) {
        //printf("Size of class_labels is %d\n", class_labels.size());
        //class_labels[cc];
        printf("Creating Fisher classifier for class %2d, %s", cc, class_labels[cc].c_str());

        // 3. Sort images into arrays for each class
        vector<int> idx_class0, idx_class1;
        for (int ii = 0; ii < images.size(); ii++) {
            if (class_ids[ii] == cc)
                idx_class1.push_back(ii);
            else
                idx_class0.push_back(ii);
        }
        int size_class0 = idx_class0.size();
        int size_class1 = idx_class1.size();
        printf(", %3d vs %3d\n", size_class0, size_class1);
        assert(size_class0 > 0 && size_class1 > 0);
        
        //ImageInfo(images[idx_class0[0]]);
        //ImageInfo(images[idx_class1[0]]);

        // Here we copy features into two separate matrices, one for each class
        CvMat* features_class0 = cvCreateMat(size_class0, eigen_dimensions, CV_32F);
        CvMat* features_class1 = cvCreateMat(size_class0, eigen_dimensions, CV_32F);
        for (int ii = 0; ii < size_class0; ii++)
            for (int jj = 0; jj < eigen_dimensions; jj++)
                CV_MAT_ELEM(*features_class0, float, ii, jj) = CV_MAT_ELEM(*features, float, idx_class0[ii], jj);
        for (int ii = 0; ii < size_class1; ii++)
            for (int jj = 0; jj < eigen_dimensions; jj++)
                CV_MAT_ELEM(*features_class1, float, ii, jj) = CV_MAT_ELEM(*features, float, idx_class1[ii], jj);


        // 4. compute covariance matrices and means for class 0 (not
        // selected) and class 1 (selected)
        CvMat* cov0 = cvCreateMat(eigen_dimensions, eigen_dimensions, CV_32F);
        CvMat* cov1 = cvCreateMat(eigen_dimensions, eigen_dimensions, CV_32F);

        CvMat* mean0 = cvCreateMat(1, eigen_dimensions, CV_32F);
        CvMat* mean1 = cvCreateMat(1, eigen_dimensions, CV_32F);

        //cvCalcCovarMatrix((const CvArr**)&features_class0, size_class0, cov0, mean0, CV_COVAR_ROWS);
        //cvCalcCovarMatrix((const CvArr**)&features_class1, size_class1, cov1, mean1, CV_COVAR_ROWS);
        cvCalcCovarMatrix((const CvArr**)&features_class0, size_class0, cov0, mean0, CV_COVAR_ROWS | CV_COVAR_NORMAL);
        cvCalcCovarMatrix((const CvArr**)&features_class1, size_class1, cov1, mean1, CV_COVAR_ROWS | CV_COVAR_NORMAL);


        // 5. sum + white noise
        CvMat* covs_whitened = cvCreateMat(eigen_dimensions, eigen_dimensions, CV_32F);
        cvAdd(cov0, cov1, covs_whitened);
        for(int ii = 0; ii < eigen_dimensions; ii++)
            CV_MAT_ELEM(*covs_whitened, float, ii, ii) += epsilon;
        
        // 6. inverse
        CvMat* covs_inv = cvCreateMat(eigen_dimensions, eigen_dimensions, CV_32F);
        cvInvert(covs_whitened, covs_inv);

        // 7. subtract means (mean01 = mean1 - mean0)
        CvMat* mean01 = cvCreateMat(1, eigen_dimensions, CV_32F);
        cvAddWeighted(mean0, -1, mean1, 1, 0, mean01);

        // 8. compute w = inv_covs * (u_1 - u_0)
        CvMat* weight = cvCreateMat(1, eigen_dimensions, CV_32F);
        cvMatMul(mean01, covs_inv, weight);

        // 9. compute fisher scores for u_0 and u_1
        scores_mean0.push_back(cvDotProduct(weight, mean0));
        scores_mean1.push_back(cvDotProduct(weight, mean1));

        // cout << "Class " << cc << " weight vector is [ ";
        // for (int jj = 0; jj < eigen_dimensions; jj++)
        //     cout << CV_MAT_ELEM(*weight, float, 0, jj) << " " << endl;
        // cout << "]" << endl;

        weights.push_back(weight);
    }



    /******************************
     *  4. Classify given photo
     ******************************/

    IplImage* test_image = cvLoadImage(image_to_classify, CV_LOAD_IMAGE_GRAYSCALE);

    CvMat* test_features = cvCreateMat(1, eigen_dimensions, CV_32F);

    float temp[eigen_dimensions];
    cvEigenDecomposite(test_image,               // image to project
                       eigen_dimensions,         // number of eigen vectors
                       (void*)eigen_array,       // eigen_vectors
                       0, 0,                     // ioflags, user callback data
                       eigen_images[0],          // average image = first eigen_vector
                       temp);                    // output

    for (size_t jj = 0; jj < eigen_dimensions; jj++)
        CV_MAT_ELEM(*test_features, float, 0, jj) = temp[jj];


    vector<float> scores;
    float max_score = -1e100;
    int max_idx = -1;
    //printf("%d ", eigen_dimensions);
    for (int ii = 0; ii < num_classes; ii++) {
        float test_score = cvDotProduct(weights[ii], test_features);

        float ratio_score = (test_score - scores_mean0[ii]) / (scores_mean1[ii] - scores_mean0[ii]);

        printf("Class %d, %s from %f to %f, score is %f, ratio %f\n",
               ii, class_labels[ii].c_str(), scores_mean0[ii],
               scores_mean1[ii], test_score, ratio_score);
        
        //printf("%f ", ratio_score);

        scores.push_back(ratio_score);

        if (ratio_score > max_score) {
            max_score = ratio_score;
            max_idx = ii;
        }
    }
    //printf("\n");
    

    printf("Best class is %d, %s\n", max_idx, class_labels[max_idx].c_str());



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
                //printf("%.2f", CV_IMAGE_ELEM(image, float, ii, jj*nChannels+cc));
                printf("%d", CV_IMAGE_ELEM(image, char, ii, jj*nChannels+cc));
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
