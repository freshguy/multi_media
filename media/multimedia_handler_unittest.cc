// Copyright (c) 2013 Juyou Inc. All rights reserved.
// Author: wangjuntao@juyou.com (Juntao Wang)

#include <string>
#include <vector>
#include "base/logging.h"
#include "base/flags.h"
#include "news_search/base/timer/timer.h"
#include "file/file.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "juyou/multi_media/media/multimedia_handler.h"

namespace juyou {
namespace multimedia {

TEST(MultiMediaHandler, MultiMediaHandler_HandleProc) {
  std::vector<std::string> paramSegments;
  paramSegments.push_back("-i");
  paramSegments.push_back("/tmp/MEDIA_6b8f158bb54045658ac3f5d2833cb5e8/38c5a2d0-3602-4953-9066-5d3cfaba43b5_headPad.wav");
  paramSegments.push_back("-t");
  paramSegments.push_back("15");
  paramSegments.push_back("-c");
  paramSegments.push_back("copy");
  paramSegments.push_back("/tmp/MEDIA_6b8f158bb54045658ac3f5d2833cb5e8/38c5a2d0-3602-4953-9066-5d3cfaba43b5_tail.wav");
  MultiMediaHandler::HandleFFmpeg(paramSegments);

#if 0
  paramSegments.clear();
  paramSegments.push_back("-r");
  paramSegments.push_back("50");
  paramSegments.push_back("-f");
  paramSegments.push_back("image2");
  paramSegments.push_back("-i");
  paramSegments.push_back("/home/taogle/Desktop/gif/frames/%d.jpg");
  paramSegments.push_back("-strict");
  paramSegments.push_back("-2");
  paramSegments.push_back("/home/taogle/Desktop/gif/frames/mymovie.mp4");
  MultiMediaHandler::HandleFFmpeg(paramSegments);
#endif
}

TEST(MultiMediaHandler, MultiMediaHandler_HandleAudio) {
  LOG(INFO) << "----------------------------------------------------";
  std::vector<std::string> paramSegments;
  paramSegments.push_back("/home/taogle/Desktop/judian/Accurate_00.WAV");
  paramSegments.push_back("/home/taogle/Desktop/judian/Accurate_00_pad3-3.WAV");
  paramSegments.push_back("pad");
  paramSegments.push_back("3");
  paramSegments.push_back("3");
  MultiMediaHandler::HandleSox(paramSegments);
}
}  // namespace multimedia
}  // namespace juyou
