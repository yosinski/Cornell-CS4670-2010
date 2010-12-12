#include <N900Helpers.h>
#include "Framebuffer.h"
#include <opencv/cv.h>
#include <opencv/highgui.h>

IplImage *
capture_yuv24 (const FCam::Frame & frame)
{
  const FCam::Image & image = frame.image();
  IplImage * img = cvCreateImage (cvSize(image.width(), image.height()),
                          IPL_DEPTH_32F, 3);
  for (unsigned int i = 0; i < image.height(); ++i)
  {
    for (unsigned int j = 0; j < image.width(); j += 2)
    {
      unsigned char * pixels = image(j, i);
      CV_IMAGE_ELEM (img, float, i, j*3) = pixels[1]/255.0;
      CV_IMAGE_ELEM (img, float, i, j*3 + 1) = pixels[2]/255.0;
      CV_IMAGE_ELEM (img, float, i, j*3 + 2) = pixels[0]/255.0;
      CV_IMAGE_ELEM (img, float, i, j*3 + 3) = pixels[3]/255.0;
      CV_IMAGE_ELEM (img, float, i, j*3 + 4) = pixels[2]/255.0;
      CV_IMAGE_ELEM (img, float, i, j*3 + 5) = pixels[0]/255.0;
    }
  }
  return img;
}

void render_yuv24 (IplImage * img)
{
  if (img->width != 640 || img->height != 480)
  {
    return;
  }
  for (int i = 0; i < img->height; ++i)
  {
    for (int j = 0; j < img->width; j += 2)
    {
      float * pixels = &CV_IMAGE_ELEM (img, float, i, j*3);
      unsigned char * dest = (unsigned char *)Framebuffer::GetInstance().GetBuffer();
      dest[i*img->width*2 + j*2] = pixels[2]*255;
      dest[i*img->width*2 + j*2 + 1] = pixels[0]*255;;
      dest[i*img->width*2 + j*2 + 2] = pixels[1]*255;;
      dest[i*img->width*2 + j*2 + 3] = pixels[3]*255;;
    }
  }
}

void save_yuv24 (IplImage * img, const std::string & fname)
{
  cvCvtColor (img, img, CV_YCrCb2BGR);
  cvConvertScale (img, img, 255, 0);
  cvSaveImage (fname.c_str(), img);
}

IplImage *
capture_gray (const FCam::Frame & frame)
{
  const FCam::Image & image = frame.image();
  IplImage * img = cvCreateImage (cvSize(image.width (), image.height ()),
                   IPL_DEPTH_32F, 1);
  for (unsigned int i = 0; i < image.height(); ++i)
  {
    for (unsigned int j = 0; j < image.width(); j += 2)
    {
      unsigned char * pixels = image (j, i);
      CV_IMAGE_ELEM (img, float, i, j) = pixels[1]/255.0;
      CV_IMAGE_ELEM (img, float, i, j + 1) = pixels[3]/255.0;
    }
  }
  return img;
}

void render_gray (IplImage * img)
{
  if (img->width != 640 || img->height != 480)
  {
    return;
  }
  for (int i = 0; i < img->height; ++i)
  {
    for (int j = 0; j < img->width; ++j)
    {
      float * pixels = &CV_IMAGE_ELEM (img, float, i, j);
      unsigned char * dest = (unsigned char *)Framebuffer::GetInstance().GetBuffer();
      dest[i*img->width*2 + j*2] = 128;
      dest[i*img->width*2 + j*2 + 1] = pixels[0]*255;
    }
  }
}

void save_gray (IplImage * img, const std::string & fname)
{
  cvConvertScale (img, img, 255, 0);
  cvSaveImage (fname.c_str(), img);
}

IplImage *
capture_bgr24 (const FCam::Frame & frame)
{
  IplImage * img = capture_yuv24 (frame);
  cvCvtColor (img, img, CV_YCrCb2BGR);
  return img;
}

void render_bgr24 (IplImage * img)
{
  cvCvtColor (img, img, CV_BGR2YCrCb);
  render_yuv24 (img);
  return;
}

void save_bgr24 (IplImage * img, const std::string & fname)
{
  cvConvertScale (img, img, 255, 0);
  cvSaveImage (fname.c_str(), img);
}
