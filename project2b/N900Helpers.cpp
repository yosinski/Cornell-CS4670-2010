#include <N900Helpers.h>
#include <opencv/cv.h>
#include <opencv/highgui.h>

IplImage *
capture_yuv24 ( const FCam::Frame & frame )
{
  const FCam::Image & image = frame.image ();
  IplImage * img = cvCreateImage ( cvSize ( image.width (), image.height () ),
                                   IPL_DEPTH_8U, 3 );

  for ( unsigned int i = 0; i < image.height (); ++i )
  {
    for ( unsigned int j = 0; j < image.width (); j += 2 )
    {
      unsigned char * pixels = image ( j, i );
      CV_IMAGE_ELEM ( img, uchar, i, j * 3 ) = pixels[1];
      CV_IMAGE_ELEM ( img, uchar, i, j * 3 + 1 ) = pixels[2];
      CV_IMAGE_ELEM ( img, uchar, i, j * 3 + 2 ) = pixels[0];
      CV_IMAGE_ELEM ( img, uchar, i, j * 3 + 3 ) = pixels[3];
      CV_IMAGE_ELEM ( img, uchar, i, j * 3 + 4 ) = pixels[2];
      CV_IMAGE_ELEM ( img, uchar, i, j * 3 + 5 ) = pixels[0];
    }
  }
  return img;
}

void render_yuv24 ( IplImage *  )
{
/*    if (img->width != 640 || img->height != 480)
 *  {
 *      return;
 *  }
 * for (int i = 0; i < img->height; ++i)
 * {
 *  for (int j = 0; j < img->width; j += 2)
 *  {
 *    unsigned char * pixels = &CV_IMAGE_ELEM (img, uchar, i, j*3);
 *    char * dest = (char *)Framebuffer::GetInstance().GetBuffer();
 *    dest[i*img->width*2 + j*2] = pixels[1];
 *    dest[i*img->width*2 + j*2 + 1] = pixels[0];
 *    dest[i*img->width*2 + j*2 + 2] = pixels[2];
 *    dest[i*img->width*2 + j*2 + 3] = pixels[3];
 *  }
 * }
 */}

void save_yuv24 ( IplImage * img, const std::string & fname )
{
  cvCvtColor ( img, img, CV_YCrCb2RGB );
  cvSaveImage ( fname.c_str (), img );
}

IplImage *
capture_gray ( const FCam::Frame & frame )
{
  const FCam::Image & image = frame.image ();
  IplImage * img = cvCreateImage ( cvSize ( image.width (), image.height () ),
                                   IPL_DEPTH_8U, 1 );

  for ( unsigned int i = 0; i < image.height (); ++i )
  {
    for ( unsigned int j = 0; j < image.width (); j += 2 )
    {
      unsigned char * pixels = image ( j, i );
      CV_IMAGE_ELEM ( img, uchar, i, j ) = pixels[1];
      CV_IMAGE_ELEM ( img, uchar, i, j + 1 ) = pixels[3];
    }
  }
  return img;
}

void render_gray ( IplImage *  )
{
/*    if (img->width != 640 || img->height != 480)
 *  {
 *      return;
 *  }
 * for (int i = 0; i < img->height; ++i)
 * {
 *  for (int j = 0; j < img->width; ++j)
 *  {
 *    unsigned char * pixels = &CV_IMAGE_ELEM (img, uchar, i, j);
 *    char * dest = (char *)Framebuffer::GetInstance().GetBuffer();
 *    dest[i*img->width*2 + j*2] = 128;
 *    dest[i*img->width*2 + j*2 + 1] = pixels[0];
 *  }
 * }
 */}

void save_gray ( IplImage * img, const std::string & fname )
{
  cvSaveImage ( fname.c_str (), img );
}

IplImage *
capture_rgb24 ( const FCam::Frame & frame )
{
  /*FCam::Image image = demosaic (frame);
   * IplImage * img = cvCreateImage (cvSize(image.width(), image.height()),
   *                      IPL_DEPTH_8U, 3);
   * for (unsigned int i = 0; i < image.height(); ++i)
   * {
   * for (unsigned int j = 0; j < image.width(); ++j)
   * {
   *  unsigned char * pixels = image(j, i);
   *  CV_IMAGE_ELEM (img, uchar, i, j*3) = pixels[0];
   *  CV_IMAGE_ELEM (img, uchar, i, j*3 + 1) = pixels[1];
   *  CV_IMAGE_ELEM (img, uchar, i, j*3 + 2) = pixels[2];
   * }
   * }
   * return img;*/
  IplImage * img = capture_yuv24 ( frame );

  cvCvtColor ( img, img, CV_YCrCb2RGB );
  return img;
}

void render_rgb24 ( IplImage * img )
{
  cvCvtColor ( img, img, CV_RGB2YCrCb );
  render_yuv24 ( img );
  return;
  /*if (img->width != 640 || img->height != 480)
   * {
   *  return;
   * }
   * IplImage * foo = cvCreateImage (cvSize(640,480), IPL_DEPTH_8U, 1);
   * printf ("%d %d\n", img->width, img->height);
   * cvSplit (img, foo, 0, 0, 0);
   * render_gray (foo);
   * cvReleaseImage (&foo);
   * return;
   * for (int i = 0; i < img->height; ++i)
   * {
   * for (int j = 0; j < img->width; ++j)
   * {
   *  unsigned char * pixels = &CV_IMAGE_ELEM (img, uchar, i, j*3);
   *  char * dest = (char *)Framebuffer::GetInstance().GetBuffer();
   *  dest[i*img->width*4 + j*4] = pixels[0];
   *  dest[i*img->width*4 + j*4 + 1] = pixels[1];
   *  dest[i*img->width*4 + j*4 + 2] = pixels[2];
   *  dest[i*img->width*4 + j*4 + 3] = 0;
   * }
   * }*/
}

void save_rgb24 ( IplImage * img, const std::string & fname )
{
  //cvCvtColor (img, img, CV_RGB2BGR);
  cvSaveImage ( fname.c_str (), img );
}
