// Copyright (c) 2013 Juyou Inc. All rights reserved.
// Author: wangjuntao@juyou.com (Juntao Wang)
//
#include <string>
#include <vector>
#include "base/logging.h"
#include "base/flags.h"
#include "base/thrift.h"
#include "file/file.h"
#include "base/time.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "juyou/base/base64/ZBase64.h"
#include "juyou/storage/proto/gen-cpp/FileTransferServiceServlet.h"
#include "juyou/multi_media/public/multimedia_transform.h"
#include "juyou/circles/public/utility.h"

#define SERVER_HOST "42.96.174.198"
#define SERVER_PORT 12000
#define GIF_TFS_KEY "T1pRhTBb_j1RCvBVdK"

#define LOCAL_WRITE_FILE "/home/taogle/Desktop/test.gif"

DEFINE_string(remote_file_name, "T1XyETB4VT1RCvBVdK",
      "The remote file name");

using boost::shared_ptr;
using ::juyou::storage::FileTransferServiceServletClient;  // NOLINT

namespace juyou {
namespace multimedia {

#if 0
  /// Pass the unittest
TEST(MultiMediaTransform, Gif2IplImage) {
  boost::shared_ptr<base::ThriftClient<FileTransferServiceServletClient> >
    file_transfer_servlet_client_ = boost::shared_ptr<base::ThriftClient<
    FileTransferServiceServletClient> >(new base::ThriftClient<
          FileTransferServiceServletClient>(SERVER_HOST, SERVER_PORT));
 
  bool isOk = true;
  std::string remoteGifContent;
  try {
    file_transfer_servlet_client_.get()->GetService()->FileRead(
          remoteGifContent, GIF_TFS_KEY);
  } catch(const ::apache::thrift::TException& err) {
    LOG(ERROR) << "Get remoteGifContent error when gtest::Gif2IplImage "
      << "with exception: " << err.what();
    isOk = false;
  } catch(...) {
    LOG(ERROR) << "Get remoteGifContent error when gtest::Gif2IplImage";
    isOk = false;
  }
  CHECK(isOk) << "Get remoteGifContent error";
  std::string typeName, origLocalFileName, actualRemoteFileContent;
  CHECK(ExtractActualRemoteFileInfo(remoteGifContent,
          typeName, origLocalFileName, actualRemoteFileContent))
    << "Extract Actual remoteFile content error";
  CHECK("gif" == typeName)
    << "Not a gif type, typeName = " << typeName;
  int decodeOutByte = 0;
  std::string decodeGifContent = ZBase64::Decode(
        actualRemoteFileContent.c_str(),
        actualRemoteFileContent.length(), decodeOutByte);
  VLOG(4) << "Base64 Decode outByte: " << decodeOutByte;
  file::File::WriteStringToFile(decodeGifContent,
        LOCAL_WRITE_FILE);
  /// Transform the GIF data content to IplImages
  std::vector<cv::Ptr<IplImage> > multiImgs;
  CHECK(Gif2IplImages(decodeGifContent.c_str(),
          decodeGifContent.length(), multiImgs))
    << "Gif2IplImages failed";
  LOG(INFO) << "cv::IplImage count is: " << multiImgs.size();
#if 0
  std::string localGifContent;
  file::File::ReadFileToString(
        "/home/taogle/Desktop/meitu/a22c80d8jw1e9f58xbsfmg20b4069kjl.gif",
        &localGifContent);
  std::vector<cv::Ptr<IplImage> > multiImgs;
  CHECK(Gif2IplImages(localGifContent.c_str(),
          localGifContent.length(), multiImgs))
    << "Gif2IplImages failed";
  LOG(INFO) << "cv::IplImage count is: " << multiImgs.size();
#endif
}
#endif

#if 0
/// Pass the unittest
TEST(MultiMediaTransform, MultiMediaTransform_ExtractActualRemoteFileInfo) {
  boost::shared_ptr<base::ThriftClient<FileTransferServiceServletClient> >
    file_transfer_servlet_client_ = boost::shared_ptr<base::ThriftClient<
    FileTransferServiceServletClient> >(new base::ThriftClient<
          FileTransferServiceServletClient>(SERVER_HOST, SERVER_PORT));
 
  bool isOk = true;
  std::string remoteFileContent;
  try {
    file_transfer_servlet_client_.get()->GetService()->FileRead(
          remoteFileContent, FLAGS_remote_file_name);
  } catch(const ::apache::thrift::TException& err) {
    LOG(ERROR) << "Get remoteGifContent error when gtest::Gif2IplImage "
      << "with exception: " << err.what();
    isOk = false;
  } catch(...) {
    LOG(ERROR) << "Get remoteGifContent error when gtest::Gif2IplImage";
    isOk = false;
  }
  CHECK(isOk) << "Get remoteGifContent error";
  std::string typeName, origLocalFileName, actualRemoteFileContent;
  CHECK(ExtractActualRemoteFileInfo(remoteFileContent,
          typeName, origLocalFileName, actualRemoteFileContent))
    << "Extract Actual remoteFile content error";
  file::File::WriteStringToFile(actualRemoteFileContent,
        origLocalFileName);
}
#endif

TEST(MultiMediaTransform, MultiMediaTransform_Video2FrameImages) {
  std::string videoFileName = "/tmp/20131219134623.mp4";
  VideoFramesInfo videoFramesInfo;

  base::TimeTicks startT = base::TimeTicks::HighResNow();
  bool isOk =Video2FrameImagesFO(videoFileName, videoFramesInfo);
  base::TimeTicks endT = base::TimeTicks::HighResNow();
  base::TimeDelta duration = endT - startT;
  VLOG(4) << "Video2FrameImages consume " << duration.InMilliseconds() << "ms";

  CHECK(isOk) << "Failed to Video2FramesImages for videoFileName: " << videoFileName;
  VLOG(4) << "frameW = " << videoFramesInfo.frameW_
    << ", frameH " << videoFramesInfo.frameH_
    << ", fps = " << videoFramesInfo.fps_
    << ", frameCnt = " << videoFramesInfo.frameCnt_;
  
  /// Just for test
  for (int i = 0; i < videoFramesInfo.frameCnt_; i++) {
    cvSaveImage(std::string("/tmp/frames/" + IntToString(i) + ".jpg").c_str(),
          videoFramesInfo.frameImgs_[i]);
  }
  videoFramesInfo.VideoFrameImgsDestroy();
}

#if 0
TEST(MultiMediaTransform, ZipFileContentUnzip_test) {
  std::string zipFile = "/home/taogle/Desktop/gif/remote/res/6716737ccf414abca88e51820a7ffb42_user_1.zip";
  std::string zipFileNamePrefix = Utility::GetUuid("ZIP_");

  std::string videoFileName = "MP4.mp4";
  VideoFramesInfo videoFramesInfo;
  std::string zipFileContent;
  file::File::ReadFileToString(zipFile,
        &zipFileContent);
  std::map<std::string, std::string> fileNameContentsMap;
  bool isOk = ZipFileContentUnzip(zipFileContent, zipFileNamePrefix,
        videoFileName, videoFramesInfo,
        fileNameContentsMap,
        "/home/taogle/Desktop/gif/remote/res");
  CHECK(isOk) << "ZipFileContentUnzip failed";

  for (std::map<std::string, std::string>::iterator it = fileNameContentsMap.begin();
        it != fileNameContentsMap.end(); ++it) {
    file::File::WriteStringToFile(it->second,
          "/home/taogle/Desktop/gif/remote/res/" + it->first);
  }
  VLOG(4) << "frameW = " << videoFramesInfo.frameW_
    << ", frameH " << videoFramesInfo.frameH_
    << ", fps = " << videoFramesInfo.fps_
    << ", frameCnt = " << videoFramesInfo.frameCnt_;
  videoFramesInfo.VideoFrameImgsDestroy();
}

TEST(MultiMediaTransform, ImagesToVideo_test) {
  Image_to_video();
}
#endif
}  // namespace multimedia
}  // namespace juyou
