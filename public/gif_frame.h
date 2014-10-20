// Copyright (c) 2013 Juyou Inc. All rights reserved.
// Author: wangjuntao@juyou.com (Juntao Wang)

#ifndef JUYOU_MULTI_MEDIA_PUBLIC_GIF_FRAME_H_
#define JUYOU_MULTI_MEDIA_PUBLIC_GIF_FRAME_H_

#include "opencv2/opencv.hpp"

class GifFrame {
 public:
  GifFrame(IplImage* const im, int del);
  GifFrame(IplImage* const im, int del, int order);
  void Init();

  ~GifFrame();

  void AdjustDelay();
 
 public:
  IplImage* image;
  int delay;
  GifFrame* nextFrame;
  int playCount;

  int width;
  int height;
  int playOrder;
};
#endif  // JUYOU_MULTI_MEDIA_PUBLIC_GIF_FRAME_H_
