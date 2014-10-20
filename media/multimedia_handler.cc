// Copyright (c) 2013 Juyou Inc. All rights reserved.
// Author: wangjuntao@juyou.com (Juntao Wang)

#include "base/logging.h"
#include "juyou/multi_media/media/multimedia_handler.h"

namespace juyou {
namespace multimedia {

MultiMediaHandler::MultiMediaHandler() {
}

MultiMediaHandler::~MultiMediaHandler() {
}

void MultiMediaHandler::HandleFFmpeg(
      const std::vector<std::string>& paramSegments) {
  int argc = paramSegments.size();
  argc += 1;  // include the first executable file name "ffmpeg" parameter
  char** argv = new char*[argc];
  int i = 0;
  argv[i++] = (char*)"ffmpeg";
  for (; i < argc; i++) {
    argv[i] = (char*)paramSegments[i - 1].c_str();
    LOG(INFO) << "argv[" << i << "] = " << argv[i];
  }
  int retMain = 0;
  try {
    retMain = ffmpeg_main(argc, argv);
  } catch(const std::exception& err) {
    LOG(ERROR) << "ffmpeg_main exception when MultiMediaHandler::HandleProc: "
      << err.what();
  } catch(...) {
    LOG(ERROR) << "ffmpeg_main unkonwn exception when MultiMediaHandler::HandleProc";
  }
  delete []argv;
}
  
void MultiMediaHandler::HandleSox(
      const std::vector<std::string>& paramSegments) {
  int argc = paramSegments.size();
  argc += 1;  // include the first executable file name "sox" parameter
  char** argv = new char*[argc];
  int i = 0;
  argv[i++] = (char*)"sox";
  
  for (; i < argc; i++) {
    argv[i] = (char*)paramSegments[i - 1].c_str();
    LOG(INFO) << "argv[" << i << "] = " << argv[i];
  }
  int retMain = 0;
  try {
    retMain = sox_main(argc, argv);
  } catch(const std::exception& err) {
    LOG(ERROR) << "sox_main exception when MultiMediaHandler::HandleAudio: "
      << err.what();
  } catch(...) {
    LOG(ERROR) << "sox_main unknown exception when MultiMediaHandler::HandleAudio";
  }
  delete []argv;
}
}  // multimedia
}  // juyou
