// Copyright (c) 2013 Juyou Inc. All rights reserved.
// Author: wangjuntao@juyou.com (Juntao Wang)

#include "juyou/multi_media/public/gif_decoder.h"

GifDecoder::GifDecoder() {
  Init();
}

GifDecoder::~GifDecoder() {
}

void GifDecoder::Init() {
  loopCount = 1;
  isShow = false;
  /*block = new byte[256];*/
  blockSize = 0;
  lastDispose = 0;
  transparency = false;
  delay = 0;  // delay in milliseconds

  scaleW = -1;
  scaleH = -1;
}
