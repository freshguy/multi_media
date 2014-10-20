// Copyright (c) 2013 Juyou Inc. All rights reserved.
// Author: wangjuntao@juyou.com (Juntao Wang)

#ifndef JUYOU_MULTI_MEDIA_GIF_GIF_DECODER_H_
#define JUYOU_MULTI_MEDIA_GIF_GIF_DECODER_H_

#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <cstring>
#include <sys/stat.h>
#include <string>
#include <map>
#include "base/logging.h"
#include "base/flags.h"
#include <Magick++.h>

#ifdef __cplusplus
extern "C" {
#include "juyou/multi_media/gif/libnsgif.h"
#endif

#ifdef __cplusplus
}
#endif

struct GifFrameInfo {
  GifFrameInfo() {
    delay_ = 0;
    frame_data_ = NULL;
  }
  ~GifFrameInfo() {
  }

  void Destroy() {
    if (frame_data_) {
      delete []frame_data_;
    }
    frame_data_ = NULL;
  }

  int delay_;                  // in (ms)
  unsigned char* frame_data_;  // the same as 'gif_frame::frame_pointer'
};

class GifDecoder {
 public:
  GifDecoder(const char* path);
  GifDecoder(const std::string& gifData);
  ~GifDecoder();

  bool IsDecodeDone() {
    return isDecodeDone_;
  }

 public:
  unsigned int GifWidth() const;
  unsigned int GifHeight() const;
  unsigned int GifFrameCount() const;
  unsigned int GifFrameCountPartial() const;
  unsigned int GifLoopCount() const;
  unsigned int GifTotalDelayTime() const;
  int GifFrameDelayTime(int frameIndex);
  Magick::Image GifFrameImage(int frameIndex);

 private:
  void DecodeGifFrames();

  void AdjustDelayTime();

 private:
  unsigned char *load_file(const char *path, size_t *data_size);
  void warning(const char *context, int code);

 private:
  static gif_bitmap_callback_vt bitmap_callbacks_;
  bool isDecodeDone_;
  gif_animation gif_;
  std::string gifData_;   // GifData
  std::map<int, GifFrameInfo> index_gifframeinfo_map_;  // index -- delay(ms)
  int total_delay_time_;  // in (ms)
};

void *bitmap_create(int width, int height);
void bitmap_set_opaque(void *bitmap, bool opaque);
bool bitmap_test_opaque(void *bitmap);
unsigned char *bitmap_get_buffer(void *bitmap);
void bitmap_destroy(void *bitmap);
void bitmap_modified(void *bitmap);
#endif  // JUYOU_MULTI_MEDIA_GIF_GIF_DECODER_H_
