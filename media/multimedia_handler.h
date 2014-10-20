// Copyright (c) 2013 Juyou Inc. All rights reserved.
// Author: wangjuntao@juyou.com (Juntao Wang)

#ifndef JUYOU_MULTI_MEDIA_MEDIA_MULTIMEDIA_HANDLER_H_
#define JUYOU_MULTI_MEDIA_MEDIA_MULTIMEDIA_HANDLER_H_

#include <string>
#include <vector>

#ifdef __cplusplus
extern "C" {
#include "juyou/multi_media/media/ffmpeg.h"
#include "juyou/multi_media/media/sox/sox.h"
#endif

#ifdef __cplusplus
}
#endif

#define MAX_PARAM_LEN 128

namespace juyou {
namespace multimedia {

class MultiMediaHandler {
 public:
  MultiMediaHandler();
  ~MultiMediaHandler();

  static void HandleFFmpeg(
        const std::vector<std::string>& paramSegments);
  
  static void HandleSox(
        const std::vector<std::string>& paramSegments);
 private:
};
}  // namespace multimedia
}  // namespace juyou
#endif  // JUYOU_MULTI_MEDIA_MEDIA_MULTIMEDIA_HANDLER_H_
