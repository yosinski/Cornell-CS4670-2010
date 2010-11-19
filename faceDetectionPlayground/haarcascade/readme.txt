Hi Jason!

This sample code uses a Haar Cascade.  On my VM, the cascade files are stored in /usr/share/opencv/haarcascades/

compile:
g++ -lhighgui facedetection.cpp

run:
./a.out --cascade="/usr/share/opencv/haarcascades/haarcascade_frontalface_default.xml"

The above will run it off the webcam (link isight though usb in vmware)

Have Fun! 
-Cooper
