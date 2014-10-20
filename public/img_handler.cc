// Copyright (c) 2013 Juyou Inc. All rights reserved.
// Author: wangjuntao@juyou.com (Juntao Wang)

#include "juyou/multi_media/public/img_handler.h"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/imgproc/imgproc_c.h"

/// Cann't use
void ImgContent2IplImage(const std::string& imgContent,
      IplImage& iplImage) {
  int len = imgContent.size();
  std::vector<unsigned char> buf(len);
  memcpy(buf.data(), imgContent.data(), len);
  cv::Mat imgMat = cv::imdecode(buf, 1);
  iplImage = IplImage(imgMat);
}

Magick::Image ImgContent2Image(const std::string& imgContent) {
  Magick::Image img;
  img.read(Magick::Blob(reinterpret_cast<const void*>(
            imgContent.c_str()), imgContent.size()));
  return img;
}

Magick::Image CvToMagick(IplImage* image) {
  int w = image->width, h = image->height;
  Magick::Image mImage(Magick::Geometry(w,h), Magick::Color("transparent"));
  mImage.modifyImage();
  Magick::PixelPacket* pixels = mImage.getPixels(0,0,w,h);	

  uchar* data = (uchar*)image->imageData;
  for (int i = 0; i < h; i++) {
    for (int j = 0; j < w; j++) {
      pixels[i * w + j] = Magick::Color(data[i * image->widthStep + j * 3 + 2] * 256,
        data[i * image->widthStep + j * 3 + 1] * 256,
        data[i * image->widthStep + j * 3] * 256);
    }
  }
  mImage.syncPixels();
  return mImage;
}

IplImage* MagickToCv(const Magick::Image& image, bool grayScale) {
  int w = image.columns(), h = image.rows();
  int nChannels = grayScale ? 1 : 3;

  IplImage* cvImage = cvCreateImage(cvSize(w,h), IPL_DEPTH_8U, nChannels);

  Magick::Image img = image;
  if (grayScale) {
    img.quantizeColorSpace(Magick::GRAYColorspace);
  }
  const Magick::PixelPacket* pixels = img.getConstPixels(0, 0, w, h);

  uchar* data = (uchar*)cvImage->imageData;
  for (int i = 0; i < h; i++) {
    for (int j = 0; j < w; j++) {
      data[i * cvImage->widthStep + j * nChannels + 0] = pixels[i * w + j].blue / 256; // B
      if (!grayScale) {
        data[i * cvImage->widthStep + j * nChannels + 1] = pixels[i * w + j].green / 256; // G
        data[i * cvImage->widthStep + j * nChannels + 2] = pixels[i * w + j].red / 256; // R
      }
    }
  }
  return cvImage;
}

void GetPixelData(const std::string& imgFile,
      int*& rgbDatas, int& width, int& height) {
  IplImage* origImg = cvLoadImage(imgFile.c_str());
  VLOG(4) << "origImg.width = " << origImg->width
    << ", origImg.height = " << origImg->height
    << ", depth = " << origImg->depth
    << ", channels = " << origImg->nChannels
    << ", widthStep = " << origImg->widthStep;

  width = origImg->width, height = origImg->height;
  rgbDatas = new int[width * height];
  uchar* data = (uchar*)origImg->imageData;
  for (int i = 0; i < height; i++) {
    for (int j = 0; j < width; j++) {
      int r = (int)data[i * origImg->widthStep + j * 3 + 2];
      int g = (int)data[i * origImg->widthStep + j * 3 + 1];
      int b = (int)data[i * origImg->widthStep + j * 3];
      rgbDatas[i * width + j] = (r << 16) + (g << 8) + b;
      VLOG(5) << "r = " << r << ", g = " << g << ", b = " << b
        << ". data[" << i << ", " << j << "] = " << rgbDatas[i * width + j];
    }
  }
  cvReleaseImage(&origImg);
}

void WriteCVImg(int* const rgbDatas, const int& w,
      const int& h, const std::string& writeImgFile) {
  VLOG(4) << "w = " << w << ", h = " << h;
  int nChannels = 3;
  IplImage* cvImage = cvCreateImage(cvSize(w, h),
        IPL_DEPTH_8U, nChannels);
  uchar* data = (uchar*)cvImage->imageData;
  for (int i = 0; i < h; i++) {
    for (int j = 0; j < w; j++) {
      int r = (rgbDatas[i * w + j] >> 16) & 0xFF;
      int g = (rgbDatas[i * w + j] >> 8) & 0xFF;
      int b = rgbDatas[i * w + j] & 0xFF;
      
      /*
      /// OR use as below 
      int index = i * w + j;
      int r = *(rgbDatas + index) ;
      int g = *(rgbDatas + index) ;
      int b = *(rgbDatas + index) ;
      */
      VLOG(4) << "r = " << r << ", g = " << g << ", b = " << b;
      data[i * cvImage->widthStep + j * nChannels + 0] = b; // B
      data[i * cvImage->widthStep + j * nChannels + 1] = g; // G
      data[i * cvImage->widthStep + j * nChannels + 2] = r; // R
    }
  }
  cvSaveImage(writeImgFile.c_str(), cvImage);
} 

Magick::Image ScaleByCenter(Magick::Image* origImage, double scale,
      int backgroundWidth, int backgroundHeight,
      int origXOff, int origYOff) {
  int oW = origImage->columns();
  int oH = origImage->rows();
  origImage->scale(Magick::Geometry(oW * scale, oH * scale));
  int dx = (oW - oW * scale) / 2;
  int dy = (oH - oH * scale) / 2;
  Magick::Image canvas(
        Magick::Geometry(backgroundWidth, backgroundHeight),
        Magick::Color("transparent"));
  canvas.backgroundColor("transparent");
  canvas.composite(*origImage,
        origXOff + dx, origYOff + dy,
        Magick::OverCompositeOp);
  return canvas;
}

Magick::Image ImageRePosition(Magick::Image* origImage,
      int backgroundWidth, int backgroundHeight,
      int xOff, int yOff) {
  Magick::Image canvas(
        Magick::Geometry(backgroundWidth, backgroundHeight),
        Magick::Color("transparent"));
  canvas.backgroundColor("transparent");
  canvas.composite(*origImage,
        xOff, yOff,
        Magick::OverCompositeOp);
  return canvas;
}

Magick::Image RotateByCenter(Magick::Image* origImage,
      double degree,
      int backgroundWidth, int backgroundHeight,
      int xOff, int yOff) {
  origImage->backgroundColor("transparent");
  int oW = origImage->columns();
  int oH = origImage->rows();
  origImage->rotate(degree);
  int cW = origImage->columns();
  int cH = origImage->rows();

  int xOffn = xOff - (cW - oW) / 2;
  int yOffn = yOff - (cH - oH) / 2;
  Magick::Image canvas(
        Magick::Geometry(backgroundWidth, backgroundHeight),
        Magick::Color("transparent"));
  canvas.backgroundColor("transparent");
  canvas.composite(*origImage,
        xOffn, yOffn,
        Magick::OverCompositeOp);
  return canvas;
}

void RotateAffine(Magick::Image* origImage,
      double angle) {
#if 0
  int oW = origImage->columns();
  int oH = origImage->rows();
  double factor = 1.0;
  double sx_ = (double) (factor * cos (-angle * 2 * magickPI / 180.));
  double sy_ = (double) (factor * sin (-angle * 2 * magickPI / 180.));
  
  double rx_ = -sy_;
  double ry_ = sx_;
      // 将旋转中心移至图像中间
  double tx_ = oW * 0.5f;
  double ty_ = oH * 0.5f;
#endif
  double sx_ = cos(DegreeToRadian(angle));
  double sy_ = -sin(DegreeToRadian(angle));
  double rx_ = sin(DegreeToRadian(angle)); // DegreeToRadian(angle);
  double ry_ = cos(DegreeToRadian(angle));  // DegreeToRadian(angle);
  double tx_ = 0.0;
  double ty_ = 0.0;
  Magick::DrawableAffine rotateAffine(sx_, sy_, rx_, ry_, tx_, ty_);
  origImage->affineTransform(rotateAffine);
}

void OpencvHandleImage(const char* origRgbData,
      const int& imgWidth, const int& imgHeight,
      const int& x, const int& y,
      const int& w, const int& h,
      const double& degree, const bool& isFlipOver,
      const int& scaledW, const int& scaledH,
      char* handledRgbData) {
  IplImage* origImg = cvCreateImage(cvSize(imgWidth, imgHeight),
        IPL_DEPTH_8U, 3);  // channels = 3, have no alpha channel
  origImg->imageData = (char*)origRgbData;
  /// ***** 1. rotate *****
  IplImage* rotateImg = RotateByCenterNoTrim(origImg, degree);
  cvSaveImage("/home/taogle/Desktop/rotate.png", rotateImg);
  cvReleaseImage(&origImg);       // 1. release the origImg
  IplImage* laterImg = NULL;
  /// ***** 2. flip *****
  if (isFlipOver) {
    cv::Mat flipSrcMat(rotateImg, 0);  // 0 represent share common data buffer
    cv::Mat flipDstMat;
    cv::flip(flipSrcMat, flipDstMat, 1);  // 1: right and left mirror
    laterImg = &IplImage(flipDstMat);
    cvReleaseImage(&rotateImg);   // 2.1 release rotateImg
  } else {
    laterImg = rotateImg;
  }
  /// ***** 3. Crop *****
  CvSize size = cvSize(w, h);
  cvSetImageROI(laterImg, cvRect(x, y, size.width, size.height));  // set the srcImg's ROI
  IplImage* cropImg = cvCreateImage(size, laterImg->depth, laterImg->nChannels);
  cvCopy(laterImg, cropImg);
  cvResetImageROI(laterImg);
  if (!isFlipOver) {
    cvReleaseImage(&laterImg);  // 2.2 release the laterImg(!isFilpOver, laterImg = rotateImg)
  }
  /// ***** 4. Scale *****
  IplImage* finalImg = finalImg = cvCreateImage(cvSize(scaledW, scaledH),
        cropImg->depth, cropImg->nChannels);
  cvResize(cropImg, finalImg, CV_INTER_LINEAR);
  cvReleaseImage(&cropImg);    // 3. release the cropImg
  int handledSize = finalImg->width * finalImg->height * finalImg->nChannels;
  memmove(handledRgbData, finalImg->imageData, handledSize);
  cvReleaseImage(&finalImg);   // 4. release the finalImg
}  

IplImage* RotateByCenterNoTrim(const IplImage* img,
      const double& degree) {
  double angle = degree  * CV_PI / 180.;  // clockwise
  double a = sin(angle), b = cos(angle); 
  int width = img->width;  
  int height = img->height;  
  int width_rotate = int(height * fabs(a) + width * fabs(b));  
  int height_rotate = int(width * fabs(a) + height * fabs(b));  
  /// The rotate map
  // [ m0  m1  m2 ] ===>  [ A11  A12   b1 ]
  // [ m3  m4  m5 ] ===>  [ A21  A22   b2 ]
  float map[6];
  CvMat map_matrix = cvMat(2, 3, CV_32F, map);  
  /// The rotate center 
  CvPoint2D32f center = cvPoint2D32f(width / 2, height / 2);  
  cv2DRotationMatrix(center, -degree, 1.0, &map_matrix);  
  map[2] += (width_rotate - width) / 2;  
  map[5] += (height_rotate - height) / 2;  
  IplImage* img_rotate = cvCreateImage(cvSize(width_rotate, height_rotate), 8, 3); 
  /// Apply affine transform
  /// CV_WARP_FILL_OUTLIERS - Fill up with all output pixels
  cvWarpAffine(img, img_rotate, &map_matrix,
        CV_INTER_LINEAR | CV_WARP_FILL_OUTLIERS, cvScalarAll(0)); 
  return img_rotate;
}

IplImage* RotateByCenterNoTrim2(const IplImage* img,
      const double& degree) {
  double angle = -degree  * CV_PI / 180.; 
  double a = sin(angle), b = cos(angle); 
  int width=img->width, height=img->height;
  // new image's size after rotate
  int width_rotate= int(height * fabs(a) + width * fabs(b));  
  int height_rotate=int(width * fabs(a) + height * fabs(b));  
  IplImage* img_rotate = cvCreateImage(cvSize(width_rotate, height_rotate), img->depth, img->nChannels);  
  cvZero(img_rotate);  
  // 保证原图可以任意角度旋转的最小尺寸
  int tempLength = sqrt((double)width * width + (double)height *height) + 10;  
  int tempX = (tempLength + 1) / 2 - width / 2;  
  int tempY = (tempLength + 1) / 2 - height / 2;  
  IplImage* temp = cvCreateImage(cvSize(tempLength, tempLength), img->depth, img->nChannels);  
  cvZero(temp);  
  // 将原图复制到临时图像tmp中心
  cvSetImageROI(temp, cvRect(tempX, tempY, width, height));  
  cvCopy(img, temp, NULL);  
  cvResetImageROI(temp);  
  // rotate array map
  // [ m0  m1  m2 ] ===>  [ A11  A12   b1 ]
  // [ m3  m4  m5 ] ===>  [ A21  A22   b2 ]
  float m[6];  
  int w = temp->width;  
  int h = temp->height;  
  m[0] = b;  
  m[1] = a;  
  m[3] = -m[1];  
  m[4] = m[0];  
  // 将旋转中心移至图像中间  
  m[2] = w * 0.5f;  
  m[5] = h * 0.5f;  
  CvMat M = cvMat(2, 3, CV_32F, m);  
  cvGetQuadrangleSubPix(temp, img_rotate, &M);  
  cvReleaseImage(&temp);  
  return img_rotate;
}  

void GetRegionBoundBox(const std::vector<Point>& polygonVertexs,
      RegionBoundBox& regionBoundBox) {
  double left = polygonVertexs[0].x;
  double right = polygonVertexs[0].x;
  double top = polygonVertexs[0].y;
  double bottom = polygonVertexs[0].y;
  for (int i = 0; i < polygonVertexs.size(); i++) {
    if (polygonVertexs[i].x < left) {  // left
      left = polygonVertexs[i].x;
    }
    if (polygonVertexs[i].x > right) {  // right
      right = polygonVertexs[i].x;
    }
    if (polygonVertexs[i].y < top) {  // top
      top = polygonVertexs[i].y;
    }
    if (polygonVertexs[i].y > bottom) {  // bottom
      bottom = polygonVertexs[i].y;
    }
  }
  regionBoundBox.left_ = left;
  regionBoundBox.right_ = right;
  regionBoundBox.top_ = top;
  regionBoundBox.bottom_ = bottom;
}

bool InsidePolygon(const std::vector<Point>& polygonVertexs,
      const Point& p) {
  int nCross = 0;
  int nCount = polygonVertexs.size();
  for (int i = 0; i < nCount; i++) {
    const Point* p1 = &polygonVertexs[i];
    const Point* p2 = &polygonVertexs[(i + 1) % nCount];
    // 求解 y = p.y 与 p1p2 的交点
    if (p1->y == p2->y) { // p1p2 与 y=p0.y平行
      continue;
    }
    if (p.y < MIN(p1->y, p2->y)) {  // 交点在p1p2延长线上
      continue;
    }
    if (p.y >= MAX(p1->y, p2->y)) {  // 交点在p1p2延长线上 
      continue;
    }
    // 求交点的 X 坐标
    double x = (double)(p.y - p1->y) * (double)(p2->x - p1->x) / (double)(p2->y - p1->y) + p1->x;
    if ( x > p.x ) {
      nCross++; // 只统计单边交点 
    }
  }
  // 单边交点为偶数，点在多边形之外
  return (nCross % 2 == 1);
}

bool InsidePolygonExII(const std::vector<Point>& polygonVertexs,
      const Point& p) {
  int verticesCount = polygonVertexs.size();
  bool isIntersectOnSection = false;

  int nCross = 0;
  for (int i = 0; i < verticesCount; ++i) {
    const Point* p1 = &polygonVertexs[i];
    const Point* p2 = &polygonVertexs[(i + 1) % verticesCount];

    // 求解 y=p.y 与 p1 p2 的交点
    if (p1->y == p2->y) {   // p1p2 与 y=p0.y平行
      continue;
    }
    if (p.y < fminf(p1->y, p2->y)) { // 交点在p1p2延长线上
      continue;
    }
    if (p.y > fmaxf(p1->y, p2->y)) { // 交点在p1p2延长线上
      continue;
    }
    // 求交点的 X 坐标
    double x =
      (double)(p.y - p1->y) * (double)(p2->x - p1->x) / (double)(p2->y - p1->y) + p1->x;
    if (fabs(x - p.x) < 10e-4) {
      /// 点正好相交在多边形的边上
      isIntersectOnSection = true;
      break;
    }
    if (x > p.x) {  // 只统计单边交点
      nCross++;
    }
  }
  if(isIntersectOnSection || nCross % 2 == 1) {
    /// 单边交点为偶数，点在多边形之外
    return true;
  } else {
    return false;
  }
}

float getAngleAOB(const Point& pA,
      const Point& pO, const Point& pB) {
  float oa = sqrtf(powf((pA.y - pO.y), 2) + powf((pA.x - pO.x), 2));
  float ob = sqrtf(powf((pB.y - pO.y), 2) + powf((pB.x - pO.x), 2));
  float ab = sqrtf(powf((pA.y - pB.y), 2) + powf((pA.x - pB.x), 2));

  float cos_AOB = (oa*oa + ob*ob - ab*ab) / (2 * oa * ob);
  return acosf(cos_AOB);
}

bool IsOnLineAB(const Point& po,
      const Point& pa, const Point& pb) {
  if (po.x >= fminf(pa.x, pb.x)
        && po.x <= fmaxf(pa.x, pb.x)
        && po.y >= fminf(pa.y, pb.y)
        && po.y <= fmaxf(pa.y, pb.y)) {
    float area1 = (pa.x - po.x) * (pb.y - po.y);
    float area2 = (pb.x - po.x) * (pa.y - po.y);

    if (fabsf(area1-area2) < MIN_NUMBER) {
      return true;
    }
    return false;
  } else {
    return false;
  }
}

/// The lowest effective
bool InsidePolygonByAngle(const std::vector<Point>& polygonVertexs,
      const Point& p) {
  float sumAngle = 0;
  bool isOnLine =false;

  int nCount = polygonVertexs.size();
  for (int i = 0; i < nCount; ++i) {
    const Point* p1 = &polygonVertexs[i];
    const Point* p2 = &polygonVertexs[(i + 1) % nCount];

    /// 首先判断是否在线段上
    if (IsOnLineAB(p, *p1, *p2)) {
      isOnLine = true;
      break;
    }
    sumAngle += getAngleAOB(*p1, p, *p2);
  }

  /// 如果角度和等于360度则在内部，否则在外部 / 在多边形上
  if(isOnLine || (fabsf(sumAngle - M_PI * 2) < MIN_NUMBER)) {
    return true;
  }
  return false;
}

bool InsidePolygonS1(const std::vector<Point>& polygonVertexs,
      const Point& p) {
  int i, j = polygonVertexs.size() - 1;
  bool oddNodes = false;
  for (i = 0; i < polygonVertexs.size(); i++) {
    if ((polygonVertexs[i].y < p.y && polygonVertexs[j].y >= p.y
            || polygonVertexs[j].y < p.y && polygonVertexs[i].y >= p.y)
          && (polygonVertexs[i].x <= p.x || polygonVertexs[j].x <= p.x)) {
      if (polygonVertexs[i].x + (p.y - polygonVertexs[i].y) / (polygonVertexs[j].y - polygonVertexs[i].y) * (polygonVertexs[j].x - polygonVertexs[i].x) < p.x) {
        oddNodes =! oddNodes;
      }
    }
    j = i;
  }
  return oddNodes;
}

bool InsidePolygonS2(const std::vector<Point>& polygonVertexs,
      const Point& p) {
  int i, j = polygonVertexs.size() - 1;
  bool oddNodes = false;
  for (i = 0; i < polygonVertexs.size(); i++) {
    if ((polygonVertexs[i].y < p.y && polygonVertexs[j].y >= p.y
            || polygonVertexs[j].y < p.y && polygonVertexs[i].y >= p.y)
          && (polygonVertexs[i].x <= p.x || polygonVertexs[j].x <= p.x)) {
      oddNodes ^= (polygonVertexs[i].x + (p.y - polygonVertexs[i].y) / (polygonVertexs[j].y - polygonVertexs[i].y) * (polygonVertexs[j].x - polygonVertexs[i].x) < p.x);
    }
    j = i;
  }
  return oddNodes;
}

void ImgClipPath(const std::vector<Point> &polygonVertexs,
      Magick::Image& image, Magick::Color color) {
  RegionBoundBox regionBoundBox;
  GetRegionBoundBox(polygonVertexs, regionBoundBox);
  VLOG(4) << "left: " << regionBoundBox.left_
    << ", right: " << regionBoundBox.right_
    << ", top: " << regionBoundBox.top_
    << ", bottom: " << regionBoundBox.bottom_;
  for (int i = 0; i < image.columns(); i++) {
    for (int j = 0; j < image.rows(); j++) {
      if (i < regionBoundBox.left_
            || i > regionBoundBox.right_
            || j < regionBoundBox.top_
            || j > regionBoundBox.bottom_) {
        image.pixelColor(i, j, color);
        continue;
      }
      if (!InsidePolygonS2(polygonVertexs, Point(i, j))) {
        image.pixelColor(i, j, color); 
      }
    }
  }
  image.backgroundColor(color);
}

Magick::Image ImageIncludeAlpha(const std::string& imgFileName,
      const std::string& imgFileContent) {
  Magick::Image baseImg = Magick::Image(Magick::Blob(
          reinterpret_cast<const void*>(imgFileContent.c_str()),
          imgFileContent.length()));
  baseImg.backgroundColor("transparent");
  /// Get the imgFile's extention
  VLOG(4) << "imgFileName: " << imgFileName;
  std::string fileExtention = imgFileName.substr(
        imgFileName.rfind('.') + 1);
  VLOG(4) << "fileExtention: " << fileExtention;
  if ("png" == fileExtention) {
    return baseImg;
  }
  Magick::Image canvas(
        Magick::Geometry(baseImg.columns(), baseImg.rows()),
        Magick::Color("transparent"));
  canvas.backgroundColor("transparent");
  canvas.composite(baseImg, 0, 0,
        Magick::OverCompositeOp);
  return canvas;
}
