// Copyright (c) 2013 Juyou Inc. All rights reserved.
// Author: wangjuntao@juyou.com (Juntao Wang)

#ifndef JUYOU_MULTI_MEDIA_PUBLIC_IMG_HANDLER_H_
#define JUYOU_MULTI_MEDIA_PUBLIC_IMG_HANDLER_H_

//#include <cv.h>
#include <Magick++.h>
#include "opencv2/highgui/highgui_c.h"
#include "opencv2/highgui/highgui.hpp"
#include <cmath>
#include <string>
#include <vector>
#include "base/logging.h"
#include "base/flags.h"
#include "file/file.h"

#include "juyou/third_party/CImg/CImg.h"
using namespace cimg_library;  // NOLINT

//#define MIN(x, y) (x < y ? x : y)
//#define MAX(x, y) (x > y ? x : y)

struct RegionBoundBox {
  RegionBoundBox() {
    left_ = 0.0;
    right_ = 0.0;
    top_ = 0.0;
    bottom_ = 0.0;
  }
  double left_;
  double right_;
  double top_;
  double bottom_;
};

class Point {
 public:
  Point(double _x, double _y) {
    x = _x;
    y = _y;
  }
  Point(const Point& copy) {
    x = copy.x;
    y = copy.y;
  }

  Point& operator=(const Point& copy) {
    if (this != &copy) {
      x = copy.x;
      y = copy.y;
    }
    return *this;
  }

 public:
  double x;
  double y;
};


#define magickPI  3.14159265358979323846264338327950288419716939937510
#define DegreeToRadian(x)  (magickPI*(x)/180.0)
#define MIN_NUMBER (10e-4)

void ImgContent2IplImage(const std::string& imgContent,
      IplImage& iplImage);

Magick::Image ImgContent2Image(const std::string& imgContent);

Magick::Image CvToMagick(IplImage* image);
IplImage* MagickToCv(const Magick::Image& image, bool grayScale = false);


void GetPixelData(const std::string& imgFile,
      int*& rgbDatas, int& width, int& height);

void WriteCVImg(int* const rgbDatas, const int& w,
      const int& h, const std::string& writeImgFile);

/// origImage is the original image (xOff_ = 0, yOff_ = 0)
Magick::Image ScaleByCenter(Magick::Image* origImage, double scale,
      int backgroundWidth = 480, int backgroundHeight = 800,
      int origXOff = 0, int origYOff = 0);

/// origImage is the original image (xOff_ = 0, yOff_ = 0)
Magick::Image ImageRePosition(Magick::Image* origImage,
      int backgroundWidth = 480, int backgroundHeight = 800,
      int xOff = 0, int yOff = 0);

Magick::Image RotateByCenter(Magick::Image* origImage,
      double degree,
      int backgroundWidth = 480, int backgroundHeight = 800,
      int xOff = 0, int yOff = 0);

void RotateAffine(Magick::Image* origImage,
      double degree);

void OpencvHandleImage(const char* origRgbData,
      const int& imgWidth, const int& imgHeight,
      const int& x, const int& y,
      const int& w, const int& h,
      const double& degree, const bool& isFlipOver,
      const int& scaledW, const int& scaledH,
      char* handledRgbData);

/// http://blog.csdn.net/xiaowei_cqu/article/details/7616044
IplImage* RotateByCenterNoTrim(const IplImage* img, const double& degree);
IplImage* RotateByCenterNoTrim2(const IplImage* img, const double& degree);

void GetRegionBoundBox(const std::vector<Point>& polygonVertexs,
      RegionBoundBox& regionBoundBox);
/*
 * http://www.cnblogs.com/mazhenyu/archive/2010/06/13/1757855.html
 * http://www.cnblogs.com/lzjsky/archive/2011/06/20/2085314.html
 */
bool InsidePolygon(const std::vector<Point>& polygonVertexs,
      const Point& point);

/// Wrong result
bool InsidePolygonExII(const std::vector<Point>& polygonVertexs,
      const Point& p);

/// http://blog.csdn.net/lanergaming/article/details/8108503
/// The lowest effective
float getAngleAOB(const Point& pA,
      const Point& pO, const Point& pB);
bool IsOnLineAB(const Point& po,
      const Point& pa, const Point& pb);
bool InsidePolygonByAngle(const std::vector<Point>& polygonVertexs,
      const Point& p);

/// http://alienryderflex.com/polygon/
/// http://blog.csdn.net/hjh2005/article/details/9244983
bool InsidePolygonS1(const std::vector<Point>& polygonVertexs,
      const Point& p);

void ImgClipPath(const std::vector<Point> &polygonVertexs,
      Magick::Image& image,
      Magick::Color color = Magick::Color("transparent"));

Magick::Image ImageIncludeAlpha(const std::string& imgFileName,
      const std::string& imgFileContent);
#endif  // JUYOU_MULTI_MEDIA_PUBLIC_IMG_HANDLER_H_
