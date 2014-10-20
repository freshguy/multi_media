// Copyright (c) 2013 Juyou Inc. All rights reserved.
// Author: wangjuntao@juyou.com (Juntao Wang)

#ifndef JUYOU_MULTI_MEDIA_PUBLIC_MULTIMEDIA_TRANSFORM_H_
#define JUYOU_MULTI_MEDIA_PUBLIC_MULTIMEDIA_TRANSFORM_H_

//#include <cv.h>
#include <FreeImage.h>
#include "base/logging.h"
#include "base/flags.h"
#include "opencv2/opencv.hpp"

//#ifdef __cplusplus
//extern "C" {
#include "juyou/multi_media/public/unzip.h"
//#endif

//#ifdef __cplusplus
//}
//#endif

namespace juyou {
namespace multimedia {

struct VideoFramesInfo {
  int frameW_;
  int frameH_;
  int fps_;
  int frameCnt_;
  std::vector<IplImage*> frameImgs_;
  void VideoFrameImgsDestroy() {
    for (int i = 0; i < frameImgs_.size(); i++) {
      if (frameImgs_[i]) {
        cvReleaseImage(&frameImgs_[i]);
        VLOG(5) << "Destroy Img for frame " << i;
      }
      frameImgs_[i] = NULL;
    }
    frameImgs_.clear();
  }
};

cv::Ptr<IplImage> FIBITMAP2IplImage(FIBITMAP* fiBmp,
      const FREE_IMAGE_FORMAT& fif);

bool Gif2IplImages(const char* data,
      const size_t dataSize,
      std::vector< cv::Ptr<IplImage> >& mutilImgs);

bool ExtractActualRemoteFileInfo(
      const std::string remoteFileContent,
      std::string& typeName,
      std::string& origLocalFileName,
      std::string& actualRemoteFileContent);

bool ZipFileContentUnzip(const std::string& zipFileContent,
      const std::string& zipFileNamePrefix,
      std::vector<std::string>& extractFileNames,
      std::string zipExtractPathPrefix = "/tmp");

bool ZipFileContentUnzip(const std::string& zipFileContent,
      const std::string& zipFileNamePrefix,
      const std::string& videoFileName,
      VideoFramesInfo& videoFramesInfo,
      std::map<std::string, std::string>& fileNameContentsMap,
      std::string pathPrefix = "/tmp");

bool ExtractFileNamesInZip(unzFile uf,
      std::vector<std::string>& zipFileNames);

bool Video2FrameImages(const std::string& videoFileName,
      VideoFramesInfo& videoFramesInfo);

bool Video2FrameImagesFO(const std::string& videoFileName,
      VideoFramesInfo& videoFramesInfo,
      std::string framesParentDir = "/tmp");

bool FramesSortByFileNameASC(const std::string& frameFileName1,
      const std::string& frameFileName2);

void Image_to_video();

/// http://blog.csdn.net/ithomer/article/details/6318751
int mysystem(const char* cmdstring, char* buf, int len);
}  // namespace multimedia
}  // namespace juyou
#endif  // JUYOU_MULTI_MEDIA_PUBLIC_MULTIMEDIA_TRANSFORM_H_
