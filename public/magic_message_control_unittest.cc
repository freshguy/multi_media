// Copyright (c) 2013 Juyou Inc. All rights reserved.
// Author: wangjuntao@juyou.com (Juntao Wang)

#include <string>
#include <vector>
#include "base/logging.h"
#include "base/flags.h"
#include "file/file.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "juyou/multi_media/public/magic_message_control.h"

DEFINE_string(json_string, "{\"screenWidth\":480,\"screenHeight\":800,\"baseUrl\":\"\",\"elements\":[{\"id\":\"\",\"type\":\"audio_mediaplayer\",\"url\":\"a0b7128a-b749-4edd-9179-7a74ad93c448.Ogg\",\"isRerun\":false,\"x\":12,\"y\":665,\"w\":125,\"h\":125,\"scale\":1,\"degree\":0,\"actions\":{\"play\":[{\"startTime\":0,\"duration\":15000}]},\"visible\":true},{\"id\":\"\",\"type\":\"gif\",\"url\":\"a0b7128a-b749-4edd-9179-7a74ad93c448.gif\",\"isRerun\":false,\"x\":205,\"y\":430,\"w\":70,\"h\":90,\"scale\":1,\"degree\":0,\"actions\":{\"visibility\":[{\"startTime\":15000,\"visible\":false}]},\"visible\":true}],\"duration\":15000}",
      "The json string");

namespace juyou {
namespace multimedia {

TEST(MagicMessageControl, MagicMessageControl_construct) {
  MagicMessageControl magic_message_control;
  if (!magic_message_control.ExtractMessageControlRule(
          FLAGS_json_string)) {
    LOG(ERROR) << "ExtractMessageControlRule error for json_str: "
      << FLAGS_json_string;
  }
 
  LOG(INFO) << "-------------------- TEST -----------------------";
  std::string jsonContent;
  file::File::ReadFileToString(
        "/home/taogle/Desktop/gif/template_1.txt",
        &jsonContent);
  // LOG(INFO) << "JsonContentFromFile: " << jsonContent;
  MagicMessageControl magic_message_control1;
  if (!magic_message_control1.ExtractMessageControlRule(
          jsonContent)) {
    LOG(ERROR) << "ExtractMessageControlRule error for json_str: "
      << jsonContent;
  }
}
}  // namespace multimedia
}  // namespace juyou
