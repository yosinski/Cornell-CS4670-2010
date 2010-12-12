#ifndef N900_HELPERS_H
#define N900_HELPERS_H

#include <opencv/cv.h>
#include <FCam/N900.h>
#include <string>

IplImage * capture_yuv24 (const FCam::Frame &);
void render_yuv24 (IplImage *);
void save_yuv24 (IplImage *, const std::string &);

IplImage * capture_gray (const FCam::Frame &);
void render_gray (IplImage *);
void save_gray (IplImage *, const std::string &);

IplImage * capture_bgr24 (const FCam::Frame &);
void render_bgr24 (IplImage *);
void save_bgr24 (IplImage *, const std::string &);

#endif
