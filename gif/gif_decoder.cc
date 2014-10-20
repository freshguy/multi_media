// Copyright (c) 2013 Juyou Inc. All rights reserved.
// Author: wangjuntao@juyou.com (Juntao Wang)

#include <cv.h>
#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui_c.h"
#include "base/string_util.h"
#include "juyou/multi_media/gif/gif_decoder.h"

gif_bitmap_callback_vt GifDecoder::bitmap_callbacks_ = {
  bitmap_create,
  bitmap_destroy,
  bitmap_get_buffer,
  bitmap_set_opaque,
  bitmap_test_opaque,
  bitmap_modified
};

GifDecoder::GifDecoder(const char* path) {
  isDecodeDone_ = false;
  total_delay_time_ = 0;
  /// Create this gif animation
  gif_create(&gif_, &GifDecoder::bitmap_callbacks_);
  
  /// Load file into memory
  size_t size;
  unsigned char* gif_data_ = load_file(path, &size);  // should free manually
  if (!gif_data_) {
    LOG(ERROR) << "Null gif_data_ pointer when GifDecoder";
    isDecodeDone_ = false;
    goto end;
  }
  gifData_ = (char*)gif_data_;
 
  /// Begin decoding
  gif_result code;
  do {
    code = gif_initialise(&gif_, size, gif_data_);
    if (code != GIF_OK && code != GIF_WORKING) {
	  warning("gif_initialise", code);
      isDecodeDone_ = false;
      goto end; 
    }
  } while (GIF_OK != code);
  
  /// DecodeGifFrames
  DecodeGifFrames();
  isDecodeDone_ = true;
end:
  /// Don't forget to free gif_data_
  if (gif_data_) {
    free(gif_data_);
  }
  gif_data_ = NULL;
}

GifDecoder::GifDecoder(const std::string& gifData) {
  isDecodeDone_ = false;
  total_delay_time_ = 0;
  /// Create this gif animation
  gif_create(&gif_, &GifDecoder::bitmap_callbacks_);
  
  gifData_ = gifData;
  size_t size = gifData.size();
  unsigned char* gif_data_ = (unsigned char*)gifData.c_str();  // No need manully free
  
  /// Begin decoding
  gif_result code;
  do {
    code = gif_initialise(&gif_, size, gif_data_);
    if (code != GIF_OK && code != GIF_WORKING) {
	  warning("gif_initialise", code);
      isDecodeDone_ = false;
      goto end;
    }
  } while (GIF_OK != code);
  /// DecodeGifFrames
  DecodeGifFrames();
  isDecodeDone_ = true;
end:
  gif_data_ = NULL;
}

GifDecoder::~GifDecoder() {
  gif_finalise(&gif_);
  for (std::map<int, GifFrameInfo>::iterator it
        = index_gifframeinfo_map_.begin();
        it != index_gifframeinfo_map_.end(); ++it) {
    it->second.Destroy();
  }
}

void GifDecoder::DecodeGifFrames() {
  gif_frame* framePtr = gif_.frames;  // the first gif_frame pointer
  CHECK(framePtr) << "Null pointer for first gif frame";
  for (int i = 0; i != gif_.frame_count; i++) {
    gif_result code = gif_decode_frame(&gif_, i);
    if (code != GIF_OK) {
      warning("gif_decode_frame", code);
    }

    unsigned char* image = (unsigned char *)gif_.frame_image;
    GifFrameInfo gifframe_info;
    int bpp = GifWidth() * GifHeight() * 4;
    gifframe_info.frame_data_ = new unsigned char[bpp];
    memmove(gifframe_info.frame_data_, image, bpp);
#if 0
    /// TEST for the bitmap data
    unsigned int row, col;
    for (row = 0; row != gif_.height; row++) {
      for (col = 0; col != gif_.width; col++) {
        size_t z = (row * gif_.width + col) * 4;
        printf("%u %u %u ",
              (unsigned char) image[z],
              (unsigned char) image[z + 1],
              (unsigned char) image[z + 2]);
      }
      printf("\n");
    }
#endif
    
    int frameDelay = 10 * framePtr->frame_delay;  // 10 * cs = ms
    total_delay_time_ += frameDelay;

    gifframe_info.delay_ = frameDelay;
    index_gifframeinfo_map_.insert(std::pair<int, GifFrameInfo>(
            i, gifframe_info));
    framePtr++;
  }

  /// Adjust all frames' delayTime, avoid the odd delayTime (2013.12.12)
  AdjustDelayTime();

#if 0
  /*
   *   *** [OK] TEST Write image file ***
   * http://stackoverflow.com/questions/5781389/converting-a-binary-stream-into-a-png-format/5781625#5781625
   * http://stackoverflow.com/questions/16069407/how-to-save-cvwrite-or-imwrite-an-image-in-opencv-2-4-3/16070280#16070280
   * http://stackoverflow.com/questions/5410167/how-to-save-an-iplimage
   */
  for (int i = 0; i < gif_.frame_count; i++) {
    IplImage* img = cvCreateImage(cvSize(GifWidth(), GifHeight()),
          IPL_DEPTH_8U, 4);  // depth: IPL_DEPTH_8U, channels: 4 (don't use 3)
    img->imageData = (char*)index_gifframeinfo_map_[i].frame_data_;
    std::string fileName = IntToString(i) + ".png";
    if (!cvSaveImage(fileName.c_str(), img)) {
      LOG(ERROR) << "Write error for " << fileName;
    }
    cvReleaseImage(&img);
  }
#endif
}

void GifDecoder::AdjustDelayTime() {
  std::map<int, int> delayTimeCntMap;
  for (std::map<int, GifFrameInfo>::iterator it
        = index_gifframeinfo_map_.begin();
        it != index_gifframeinfo_map_.end(); ++it) {
    if (delayTimeCntMap.find(it->second.delay_) ==
          delayTimeCntMap.end()) {
      delayTimeCntMap.insert(std::pair<int, int>(
              it->second.delay_, 1));
    } else {
      delayTimeCntMap[it->second.delay_]++;
    }
  }
  std::map<int, int>::iterator it = delayTimeCntMap.begin();
  int mostDelay = it->first;  // mostDelay is the most delayTime in all frames
  int maxCnt = it->second;
  for (; it != delayTimeCntMap.end(); ++it) {
    if (it->second > maxCnt) {
      mostDelay = it->first;
    }
  } 
  for (std::map<int, GifFrameInfo>::iterator it_f
        = index_gifframeinfo_map_.begin();
        it_f != index_gifframeinfo_map_.end(); ++it_f) {
    it_f->second.delay_ = mostDelay;
  }
}

unsigned int GifDecoder::GifWidth() const {
  return gif_.width;
}

unsigned int GifDecoder::GifHeight() const {
  return gif_.height;
}

unsigned int GifDecoder::GifFrameCount() const {
  return gif_.frame_count;
}

unsigned int GifDecoder::GifFrameCountPartial() const {
  return gif_.frame_count_partial;
}

unsigned int GifDecoder::GifLoopCount() const {
  return gif_.loop_count;
}

unsigned int GifDecoder::GifTotalDelayTime() const {
  return total_delay_time_;
}

int GifDecoder::GifFrameDelayTime(int frameIndex) {
  if (index_gifframeinfo_map_.find(frameIndex)
        == index_gifframeinfo_map_.end()) {
    LOG(ERROR) << frameIndex << " is a invalid frame index";
    return -1;
  }
  return index_gifframeinfo_map_[frameIndex].delay_;
}

Magick::Image GifDecoder::GifFrameImage(int frameIndex) {
  /// Such as: Image image( 640, 480, "RGB", 0, pixels);
  return Magick::Image(GifWidth(),
        GifHeight(),
        "BGRA",
        Magick::CharPixel,
        (void*)index_gifframeinfo_map_[frameIndex].frame_data_);
}

unsigned char* GifDecoder::load_file(const char *path,
      size_t *data_size) {
  FILE *fd;
  struct stat sb;
  unsigned char *buffer;
  size_t size;
  size_t n;

  fd = fopen(path, "rb");
  if (!fd) {
    perror(path);
    exit(EXIT_FAILURE);
  }

  if (stat(path, &sb)) {
    perror(path);
    exit(EXIT_FAILURE);
  }
  size = sb.st_size;

  buffer = (unsigned char*)malloc(size);
  if (!buffer) {
    fprintf(stderr, "Unable to allocate %lld bytes\n",
          (long long) size);
    exit(EXIT_FAILURE);
  }

  n = fread(buffer, 1, size, fd);
  if (n != size) {
    perror(path);
    exit(EXIT_FAILURE);
  }

  fclose(fd);

  *data_size = size;
  return buffer;
}

void GifDecoder::warning(const char *context, int code) {
  std::string logStr = context;
  logStr += " failed: ";
  switch (code) {
    case GIF_INSUFFICIENT_FRAME_DATA: {
      logStr += "GIF_INSUFFICIENT_FRAME_DATA";
      break;
    }
    case GIF_FRAME_DATA_ERROR: {
      logStr += "GIF_FRAME_DATA_ERROR";
      break;
    }
    case GIF_INSUFFICIENT_DATA: {
      logStr += "GIF_INSUFFICIENT_DATA";
      break;
    }
    case GIF_DATA_ERROR: {
      logStr += "GIF_DATA_ERROR";
      break;
    }
    case GIF_INSUFFICIENT_MEMORY: {
      logStr += "GIF_INSUFFICIENT_MEMORY";
      break;
    }
    default: {
      logStr += "unknown code ";
      logStr += IntToString(code);
      break;
    }
  }
  LOG(ERROR) << logStr;
}

void *bitmap_create(int width, int height) {
  return calloc(width * height, 4);
}

void bitmap_set_opaque(void *bitmap, bool opaque) {
  (void) opaque;  // unused
  assert(bitmap);
}

bool bitmap_test_opaque(void *bitmap) {
  assert(bitmap);
  return false;
}

unsigned char *bitmap_get_buffer(void *bitmap) {
  assert(bitmap);
  return (unsigned char*)bitmap;
}

/// Note: currently free the first frame only
void bitmap_destroy(void *bitmap) {
  assert(bitmap);
  free(bitmap);
}

void bitmap_modified(void *bitmap) {
  assert(bitmap);
  return;
}
