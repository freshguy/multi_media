// Copyright (c) 2013 Juyou Inc. All rights reserved.
// Author: wangjuntao@juyou.com (Juntao Wang)

#include <string>
#include <vector>
#include "base/logging.h"
#include "base/flags.h"
#include "file/file.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "juyou/multi_media/public/zip_gif.h"

DEFINE_string(config_file, "/home/taogle/Desktop/gif/remote/res/config.xml",
      "The config xml file");
DEFINE_string(pathm_file, "/home/taogle/Desktop/gif/remote/res/pathm",
      "The pathm file");

DEFINE_string(zip_file, "/home/taogle/Desktop/gif/remote/res/6716737ccf414abca88e51820a7ffb42_user_1.zip",
      "The zip file name");

namespace juyou {
namespace multimedia {

#if 0
TEST(ZipGif, ZipGif_ParseFramesHandledParam_test) {
  ZipGif zipGif("test.zip");
  std::string xmlContent;
  file::File::ReadFileToString(FLAGS_config_file,
        &xmlContent);
  zipGif.ParseFramesHandledParam(xmlContent);
}

TEST(ZipGif, ZipGif_AddPathmGPointsMap_test) {
  ZipGif zipGif("test.zip");
  std::string pathmContent;
  file::File::ReadFileToString(FLAGS_pathm_file,
        &pathmContent);
  zipGif.AddPathmGPointsMap(FLAGS_pathm_file,
        pathmContent);
}
#endif

TEST(ZipGif, ZipGif_ParseZipFileContent_test) {
  ZipGif zipGif("test.zip");
  std::string zipFileContent;
  file::File::ReadFileToString(FLAGS_zip_file,
        &zipFileContent);
  CHECK(zipGif.ParseZipFileContent(zipFileContent))
    << "ParseZipFileContent failed";

#if 0
  /// Test the CreateDir
  std::string makeDir = "/tmp/image/haha/good";
  if (!file::File::RecursivelyCreateDir(makeDir, 0777)) {
    LOG(ERROR) << "CreateDir error for path: " << makeDir;
  }
#endif
  file::File::WriteStringToFile(" day day up!!!!!",
        "/tmp/test");
}
}  // namespace multimedia
}  // namespace juyou
