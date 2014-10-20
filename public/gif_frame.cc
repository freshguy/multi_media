// Copyright (c) 2013 Juyou Inc. All rights reserved.
// Author: wangjuntao@juyou.com (Juntao Wang)

#include "juyou/multi_media/public/gif_frame.h"

GifFrame::GifFrame(IplImage* const im, int del) {
  Init();
  image = im;
  delay = del;
  width = im->width;
  height = im->height;
  AdjustDelay();
}

GifFrame::GifFrame(IplImage* const im, int del, int order) {
  Init();
  image = im;
  delay = del;
  width = im->width;
  height = im->height;
  playOrder = order;
  AdjustDelay();
}

void GifFrame::Init() {
  nextFrame = NULL;
  playCount = -1;
  playOrder = 0;
}

GifFrame::~GifFrame() {
  if (image) {
    cvReleaseImage(&image);
  }
  image = NULL;
}

void GifFrame::AdjustDelay() {
  if (delay < 5) {
    delay = 100;
  } else if (delay < 30) {
    delay = 30;
  } else if (delay > 3000) {
    delay = 3000;
  }
}
