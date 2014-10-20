// Copyright (c) 2013 Juyou Inc. All rights reserved.
// Author: wangjuntao@juyou.com (Juntao Wang)

#include <string>
#include <vector>
#include "base/logging.h"
#include "base/flags.h"
// #include "base/thrift.h"
#include "news_search/base/timer/timer.h"
#include "file/file.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "juyou/multi_media/gif/gif_decoder.h"

// using boost::shared_ptr;
DEFINE_string(gif_file, "/home/taogle/Desktop/gif.gif",
      "The gif file name");

TEST(GifDecoder, GifDecoder_test) {
  /// Load from file
  GifDecoder gif_decoder(FLAGS_gif_file.c_str());
  LOG(INFO) << "Width: " << gif_decoder.GifWidth();
  LOG(INFO) << "Height: " << gif_decoder.GifHeight();
  LOG(INFO) << "FrameCount: " << gif_decoder.GifFrameCount();
  LOG(INFO) << "FrameCountPartial: " << gif_decoder.GifFrameCountPartial();
  LOG(INFO) << "LoopCount: " << gif_decoder.GifLoopCount();
  if (gif_decoder.IsDecodeDone()) {
    LOG(INFO) << "gif_decoder successfully";
  }
 
  /// Simulate load from memory
  std::string gifContent;
  file::File::ReadFileToString(FLAGS_gif_file, &gifContent);
  GifDecoder gif_decoder1(gifContent);
  LOG(INFO) << "[Memory]Width: " << gif_decoder1.GifWidth();
  LOG(INFO) << "[Memory]Height: " << gif_decoder1.GifHeight();
  LOG(INFO) << "[Memory]FrameCount: " << gif_decoder1.GifFrameCount();
  LOG(INFO) << "[Memory]FrameCountPartial: " << gif_decoder1.GifFrameCountPartial();
  LOG(INFO) << "[Memory]LoopCount: " << gif_decoder1.GifLoopCount();
  LOG(INFO) << "[Memory]TotalDelayTime: " << gif_decoder1.GifTotalDelayTime();
  if (gif_decoder1.IsDecodeDone()) {
    LOG(INFO) << "gif_decoder1 successfully";
  }
  for (int i = 0; i < gif_decoder1.GifFrameCount(); i++) {
    std::string frameFile = IntToString(i) + ".png";
    gif_decoder1.GifFrameImage(i).write(frameFile.c_str());
    VLOG(4) << i << "th frame delayTime: "
      << gif_decoder1.GifFrameDelayTime(i);
  }

  /// Test the Timer
  news::Timer timer;
  timer.Begin();
  usleep(120000);
  timer.End();
  LOG(INFO) << "Have sleep: " << timer.GetMs() << "ms";
  LOG(INFO) << "CurrentTimeInMs: " << news::Timer::CurrentTimeInMs();
  LOG(INFO) << "CurrentTimeInS: " << news::Timer::CurrentTimeInS();
}
