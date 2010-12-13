#include "Fisher.h"
#include "opencv/cv.h"
#include "opencv/cvaux.h"

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <float.h>
#include <limits.h>
#include <time.h>
#include <ctype.h>

using namespace std;

Fisher::Fisher()
{
  int max_eigen_dimensions = 6;
  float epsilon = 1.0;  
  
  /******************************
     *  1. Load class_*.txt files and images
     ******************************/

    //in header: vector< string >     class_labels;       // short list of class labels
    vector< IplImage* >  images;             // long list of images
    vector< int >        class_ids;          // long list of class ids
    vector< string >     image_labels;       // long list of image labels

    /* assume it is a text file containing the
       list of the single class filenames to be processed - one per line */
    num_classes = 0;  // one more than the highest numbered class
    char* subjects_filename = "class_demo.txt";
    FILE* class_all_file = fopen( subjects_filename, "rt" );
    char tmp_buf[1000+1];
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
     *  2. Compute fisher weight vector
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
    eigen_dimensions = eigen_images.size() - 1;      // first is average

    // [JBY] use this to control how many dimensions we use
    eigen_dimensions = min(max_eigen_dimensions, eigen_dimensions);

    printf("Using first %d eigen vectors\n", eigen_dimensions);

    eigen_array = (IplImage**)malloc(sizeof(IplImage*) * eigen_dimensions);
    for(int ii = 0; ii < eigen_dimensions; ii++) {
        eigen_array[ii] = eigen_images[ii+1];
    }

    // Also save eigenvectors as one huge image:
    // e.g., a 10000x15 single image
    int num_pixels = images[0]->height * images[0]->width;
    CvMat* stacked_eigen_vectors = cvCreateMat(eigen_dimensions, num_pixels, CV_32F);
    for (int ii = 0; ii < num_pixels; ii++) {
        for (int jj = 0; jj < eigen_dimensions; jj++) {
            CV_MAT_ELEM(*stacked_eigen_vectors, float, jj, ii) = \
                CV_IMAGE_ELEM(eigen_array[jj], float, 0, ii);
        }
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

        // 9. reproject and save mean class0 and class1 images
        CvMat * tmp_mat = cvCreateMat(1, num_pixels, CV_32F);

        cvMatMul(mean0, stacked_eigen_vectors, tmp_mat);
        sprintf(tmp_buf, "class_%02d_%d.jpg", cc, 0);
        reshapeAndSave(tmp_buf, tmp_mat, cvGetSize(images[0]));

        cvMatMul(mean1, stacked_eigen_vectors, tmp_mat);
        sprintf(tmp_buf, "class_%02d_%d.jpg", cc, 1);
        reshapeAndSave(tmp_buf, tmp_mat, cvGetSize(images[0]));

        cvMatMul(weight, stacked_eigen_vectors, tmp_mat);
        sprintf(tmp_buf, "class_%02d_%s.jpg", cc, "w");
        reshapeAndSave(tmp_buf, tmp_mat, cvGetSize(images[0]));

        // 10. compute fisher scores for u_0 and u_1
        scores_mean0.push_back(cvDotProduct(weight, mean0));
        scores_mean1.push_back(cvDotProduct(weight, mean1));

        // cout << "Class " << cc << " weight vector is [ ";
        // for (int jj = 0; jj < eigen_dimensions; jj++)
        //     cout << CV_MAT_ELEM(*weight, float, 0, jj) << " " << endl;
        // cout << "]" << endl;

        weights.push_back(weight);
        avgImage = eigen_images[0];
    }
}


std::string Fisher::labelFace(IplImage* face)
{
      /******************************
     *  4. Classify given photo
     ******************************/

    CvMat* test_features = cvCreateMat(1, eigen_dimensions, CV_32F);

    float temp[eigen_dimensions];
    cvEigenDecomposite(face,               // image to project
                       eigen_dimensions,         // number of eigen vectors
                       (void*)eigen_array,       // eigen_vectors
                       0, 0,                     // ioflags, user callback data
                       avgImage,          // average image = first eigen_vector
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

    printf("Best class is %d, %s\n", max_idx, class_labels[max_idx].c_str());
    return class_labels[max_idx];
}

void Fisher::reshapeAndSave(const char* filename, const CvMat* mat, CvSize destination_size)
{
    CvMat fat_header, *fat;
    fat = cvReshape(mat, &fat_header, 0, destination_size.height);

    IplImage* fooF = cvCreateImage(destination_size, IPL_DEPTH_32F, 1);
    matToImg(fat, fooF);

    IplImage* fooC = cvCreateImage(destination_size, IPL_DEPTH_8U, 1);
    autoScaleImage(fooF, fooC, 0, 255);

    cvSaveImage(filename, fooC);
    cvReleaseImage(&fooC);
    cvReleaseImage(&fooF);
}

// scale image to be from min_goal to max_goal
void Fisher::autoScaleImage (IplImage* in, IplImage* out, float min_goal, float max_goal)
{
    float min_val = 1e100, max_val = -1e100;
    for (int ii = 0; ii < in->height; ii++) {
        for (int jj = 0; jj < in->width; jj++) {
            min_val = min(min_val, CV_IMAGE_ELEM(in, float, ii, jj));
            max_val = max(max_val, CV_IMAGE_ELEM(in, float, ii, jj));
        }
    }

    float scale = max_val == min_val ? 1.0 : (max_goal - min_goal) / (max_val - min_val);
    float shift = (min_goal - min_val) * scale;
    //printf("min: %f, max: %f, scale: %f, shift: %f\n", min_val, max_val, scale, shift);
    cvConvertScale(in, out, scale, shift);
}

void Fisher::matToImg(const CvMat * in, IplImage * out)
{
    assert(in->rows == out->height);
    assert(in->cols == out->width);
    for (int ii = 0; ii < out->height; ii++) {
        for (int jj = 0; jj < out->width; jj++) {
            //printf("elem  is %f\n", CV_MAT_ELEM(*in, float, ii, jj));
            CV_IMAGE_ELEM(out, float, ii, jj) = CV_MAT_ELEM(*in, float, ii, jj);
            //printf("  now is %f\n", CV_IMAGE_ELEM(out, float, ii, jj));
        }
    }
}


