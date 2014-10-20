// Copyright (c) 2013 Juyou Inc. All rights reserved.
// Author: wangjuntao@juyou.com (Juntao Wang)

#ifndef JUYOU_MULTI_MEDIA_PUBLIC_ZIP_GIF_H_
#define JUYOU_MULTI_MEDIA_PUBLIC_ZIP_GIF_H_

#include <string>
#include <vector>
#include <map>
#include <Magick++.h>
#include "base/logging.h"
#include "base/flags.h"
#include "base/thread.h"
#include "base/mutex.h"
#include "base/thrift.h"
#include "juyou/multi_media/public/img_handler.h"
#include "juyou/multi_media/public/multimedia_transform.h"

using boost::shared_ptr;  // NOLINT

/// for fonts
DECLARE_string(fonts_install_dir);

DECLARE_string(zip_extract_tmp_dir);

#define DEFAULT_ZIP_FILE_NAME "zipgif.zip"

/// for Local mask Images dir
DECLARE_string(local_mask_imgs_dir);

namespace juyou {
namespace multimedia {

class ZipGifBase {
};

enum ZipGifType {
  VIDEO_GIF = 1,
  IMG_GIF = 2,
};

enum PlayOrder {
  POSITIVE_ORDER = 0,
  REVERSE_ORDER = 1
};

struct PointCoorRatio {
  double xR_;
  double yR_;
};

#if 0
struct GPoint {
  int x_;
  int y_;
};
#endif

struct FrameHandledParam {
  FrameHandledParam() {
    delay_ = 100;
    maskId_ = -1;
    alpha_ = 255;
  }
  std::string frameId_;
  std::string frameName_;
  int delay_;
  std::string pathmName_;  // pathm(key)
  std::vector<PointCoorRatio> mtxFrom_;
  std::vector<PointCoorRatio> mtxTo_;
  int maskId_;
  int maskXoff_;
  int maskYoff_;
  int alpha_;
};

struct MaskImgInfo {
  MaskImgInfo(const std::string& maskUrl,
        const Magick::Image& maskImg) {
    maskUrl_ = maskUrl;
    maskImg_ = maskImg;
  }

  std::string maskUrl_;
  Magick::Image maskImg_;
};

struct ZipGifFrame {
  Magick::Image frameImg_;
  int delay_;
};

class IMGGifFrame;
class ZipGif : public ZipGifBase {
 public:
  ZipGif();
  explicit ZipGif(const std::string& zipFileName);
  ~ZipGif();

  bool ParseZipFileContent(const std::string& zipFileContent);
  bool HandleFramesByHandledParam();

 public:
  /// setter and getter
  Magick::Image GetZipGifFrameImage(int frameIndex);
  int GetZipGifFrameDelay(int frameIndex);
  int GetZipGifFrameCount() const;

 private:
  bool ParseFramesHandledParam(const std::string& configXmlContent);
  bool AddPathmGPointsMap(const std::string& pathmName,
        const std::string& pathmContent);

 private:
  bool ParsePointCoorRatio(const std::string mtxStr,
        std::vector<PointCoorRatio>& pointCoorRatios);

 public:
  ZipGifType& get_zipgif_type() {
    return zipGifType_;
  }

  /// setter and getter for VIDEO_GIF (handled in multi-thread)
  VideoFramesInfo& get_video_frames_info() {
    return videoFramesInfo_;
  }

  PlayOrder& get_play_order() {
    return playOrder_;
  }

  std::vector<MaskImgInfo>& get_mask_img_infos() {
    return maskImgInfos_;
  }

  std::vector<FrameHandledParam>& get_frame_handled_params() {
    return frameHandledParams_;
  }

  std::map<std::string, std::vector<Point> > get_pathm_name_points_map() {
    return pathmNamePointsMap_;
  }

  /// setter and getter for IMG_GIF
  std::vector<IMGGifFrame>& get_imggif_frames() {
    return imgGifFrames_;
  }

 private:
  ZipGifType zipGifType_;
  std::string zipFileName_;
  /// for VIDEO_GIF
  VideoFramesInfo videoFramesInfo_;
  PlayOrder playOrder_;
  int playTimes_;
  std::vector<MaskImgInfo> maskImgInfos_;
  // std::map<std::string, FrameHandledParam> frameIdHandledParamMap_;
  std::vector<FrameHandledParam> frameHandledParams_;
  std::map<std::string, std::vector<Point> > pathmNamePointsMap_;

  std::vector<ZipGifFrame> handledZipGifFrames_;

  std::map<int, ZipGifFrame> handledZipGifFramesIdMaps_;  // for multi-thread
  
  /// for IMG_GIF
  std::vector<IMGGifFrame> imgGifFrames_;
};

class IMGGifFrame {
 public:
  IMGGifFrame() {
  }
  IMGGifFrame(const Magick::Image& image,
        const int& delay,
        const std::string& pathmName) {
    image_ = image;
    delay_ = delay;
    pathmName_ = pathmName;
  }
 public:
  Magick::Image image_;
  int delay_;
  std::string pathmName_;
};

/// for ZipGifFrameHandleThread
class ZipGifFrameHandleThread : public base::Thread {
 public:
  ZipGifFrameHandleThread(const int& imgId, const int& totalImgsCnt,
        ZipGif* zipGif,
        base::Mutex* imgsMutex,
        base::CondVar* imgsHandledCondVar,
        std::map<int, ZipGifFrame>* handledZipGifFramesIdMaps);

  virtual ~ZipGifFrameHandleThread();

  virtual void Run();

 private:
  int imgId_;
  int totalImgsCnt_;
  ZipGif* zipGif_;
  base::Mutex* imgsMutex_;
  base::CondVar* imgsHandledCondVar_;
  std::map<int, ZipGifFrame>* handledZipGifFramesIdMaps_;
};

/// for TextZipGif
class TextZipGif : public ZipGifBase {
 public:
  TextZipGif();
  virtual ~TextZipGif();
};

bool IsNumberStr(const std::string& str);
}  // namespace multimedia
}  // namespace juyou
#endif  // JUYOU_MULTI_MEDIA_PUBLIC_ZIP_GIF_H_
