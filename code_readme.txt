Face Recognition for Mobile Platforms (N900) - Project Code
CS 4670
Cooper Bills (csb88)
Jason Yosinski (jy495)

Root Directory Layout:

FaceDetectionPlayground - 
    Contains code for haar face detection (or any object,
    dependent on cascade given).

FaceRecognitionPlayground -
    Contains all of our programs to be run locally on the
    database before transfer to the phone.  Also contains 
    the database files and test algorithms for face recognition.

final_phone -
    Code for the N900.  All of the code for our stuff is
    contained in Face.* FaceDetector.* and Fisher.*
    /tester is a program that uses the above classes
    to test on a local image using a similar interface
    as the phone.

Files of Interest:
final_phone/Face.*, FaceDetector.*, Fisher.*
  - The actual phone code, with the run-time algorithms
final_phone/phonefinalimage.tar.gz
  - A zip file of pre-compiled app for the phone, with all necessary files
face*Playground/*.cpp
  - Our Test Code (see below for description, or report for more detail on what they do)

*********************************************************************
** Note: Some of our playground apps may not compile or have crippled
** functionality.  This is because things have changed over time, and
** quick hacks were implemented. 
*********************************************************************

Descriptions of Important files:

.
|-- faceDetectionPlayground
|   `-- haarcascade
|       |-- a.out
|       |-- facedetection.cpp - Harr Cascade test code.
|       |-- Makefile
|       `-- readme.txt
|
|-- faceRecognitionPlayground
|   `-- Eigenfaces
|       |-- all_files.txt
|       |-- classFiles
|       |-- createEigenvectors.cpp - Program to create eigenvector database from a given list of preprocessed images
|       |-- createTrainingData.cpp - Program that preprocesses a list of images, by finding faces and cropping, resizing, equalizing them.  Saves results to files.
|       |-- database - Directory containing all database materials/images
|       |-- demo_classes.txt - Main class image list
|       |-- demoFiles - Script to generate database file lists
|       |-- drawSquareLabelTest.cpp - Test app for labeling faces
|       |-- eigenDecomp.cpp - Test app for Decomposing using PCA (eigenfaces)
|       |-- eigenvectors.yml - Database of eigenvectors exported from createEigenVectors
|       |-- fisher.cpp - Test app for Fisher Faces
|       |-- libcxflann_i386.a - Flann libraries (not used anymore
|       |-- Makefile
|       |-- nn.yml - Nearest neighbor information exported from createEigenVectors
|       `-- yalefaces - Directory with Yale Face Database
|
|-- final_phone 
|   |-- CameraThread.cpp - Project 1 code modified for our needs.
|   |-- CameraThread.h
|   |-- CameraWidget.cpp
|   |-- CameraWidget.h
|   |-- cs4670.pro
|   |-- Face.cpp - Face class, simple basically a struct with image pointer, label, and location
|   |-- FaceDetector.cpp - Main face processing code.  Constructor loads database and creates fisher faces, functions process a raw image
|   |-- FaceDetector.h
|   |-- Face.h
|   |-- Fisher.cpp - Contains fisher face algorithm
|   |-- Fisher.h
|   |-- Framebuffer.cpp
|   |-- Framebuffer.h
|   |-- getopt.c
|   |-- getopt.h
|   |-- Makefile
|   |-- N900Helpers.cpp
|   |-- N900Helpers.h
|   |-- N900Main.cpp
|   |-- omapfb.h
|   |-- OverlayWidget.cpp
|   |-- OverlayWidget.h
|   |-- phonefinalimage.tar.gz - A zip file of all the files pulled from the phone.  This can be unziped and copied to the N900 for instant use.  No compiling necessary
|   |-- README
|   `-- tester
|       |-- all_files.txt
|       |-- class_demo.txt - list of database text files (one file per label), each containing a list of images.
|       |-- database - Directory containing the training images and database list files.  Used for fisherfaces.
|       |-- eigenvectors.yml - Eigenvectors
|       |-- haarcascade_frontalface_alt.xml - cascade file we recommend using
|       |-- labels.txt - List of labels (corresponds line by line to class_demo.txt)
|       |-- maintest.cpp - Test app for phone structure.  Uses all the same files and loading scheme.  Takes an image file as a command line argument and applies processing onto it.  Shows result in OpenCV frame.
|       |-- Makefile
`-- readme.txt

32 directories, 1686 files

