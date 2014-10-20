// Copyright (c) 2013 Juyou Inc. All rights reserved.
// Author: wangjuntao@juyou.com (Juntao Wang)

#include "news_search/base/timer/timer.h"
#include "third_party/tinyxml/tinyxml.h"
#include "base/string_util.h"
#include "juyou/circles/public/utility.h"
#include "juyou/multi_media/public/zip_gif.h"

/// for fonts
DEFINE_string(fonts_install_dir,
      "/usr/share/fonts/truetype/newfonts",
      "The install fonts dir");

DEFINE_string(zip_extract_tmp_dir, "/tmp",
      "The zip extract tmp dir");
DEFINE_string(video_file_name, "MP4.mp4",
      "The default video name in ZipGif file");

/// for Local mask Images dir
DEFINE_string(local_mask_imgs_dir,
      "/var/yr/open_magic_message_server_000_00/static/jydata/open_magic_message",
      "The local mask image dir");

namespace juyou {
namespace multimedia {

ZipGif::ZipGif() {
  zipGifType_ = VIDEO_GIF;
  playTimes_ = -1;
  zipFileName_ = DEFAULT_ZIP_FILE_NAME;
}

ZipGif::ZipGif(const std::string& zipFileName) {
  zipGifType_ = VIDEO_GIF;
  playTimes_ = -1;
  zipFileName_ = zipFileName;
}

ZipGif::~ZipGif() {
  /// Destroy the VideoFramesInfo
  videoFramesInfo_.VideoFrameImgsDestroy();
}

Magick::Image ZipGif::GetZipGifFrameImage(int frameIndex) {
  Magick::Image image;
  if (VIDEO_GIF == zipGifType_) {
    /// (TODO) use handled img data later 12.8
    // image = CvToMagick(
          // videoFramesInfo_.frameImgs_[frameIndex]);
    //image = handledZipGifFrames_[frameIndex].frameImg_;
    image = handledZipGifFramesIdMaps_[frameIndex].frameImg_;
  } else if (IMG_GIF == zipGifType_) {
    // image = imgGifFrames_[frameIndex].image_;
    image = handledZipGifFramesIdMaps_[frameIndex].frameImg_;
  }
  return image;
}

/// In (ms)
int ZipGif::GetZipGifFrameDelay(int frameIndex) {
  int frameDelay;
  if (VIDEO_GIF == zipGifType_) {
    // frameDelay = 1000 / videoFramesInfo_.fps_;
    //frameDelay = handledZipGifFrames_[frameIndex].delay_;
    frameDelay = handledZipGifFramesIdMaps_[frameIndex].delay_;
  } else if (IMG_GIF == zipGifType_) {
    // frameDelay = imgGifFrames_[frameIndex].delay_;
    frameDelay = handledZipGifFramesIdMaps_[frameIndex].delay_;
  }
  return frameDelay;
}

int ZipGif::GetZipGifFrameCount() const {
  int frameCnt = 0;
  if (VIDEO_GIF == zipGifType_) {
    // frameCnt = videoFramesInfo_.frameCnt_;
    //frameCnt = handledZipGifFrames_.size();
    frameCnt = handledZipGifFramesIdMaps_.size();
  } else if (IMG_GIF == zipGifType_) {
    // frameCnt = imgGifFrames_.size();
    frameCnt = handledZipGifFramesIdMaps_.size();
  }
  return frameCnt;
}

bool ZipGif::ParseZipFileContent(const std::string& zipFileContent) {
  if (zipFileContent.empty()) {
    LOG(ERROR) << "Empty zipFileContent when ParseZipFileContent";
    return false;
  }
  std::map<std::string, std::string> fileNameContentsMap;
  std::string zipFileNamePrefix = Utility::GetUuid("ZIP_");
  if (!ZipFileContentUnzip(zipFileContent, zipFileNamePrefix,
          FLAGS_video_file_name, videoFramesInfo_, 
          fileNameContentsMap, FLAGS_zip_extract_tmp_dir)) {
    LOG(ERROR) << "ZipFileContentUnzip error when ParseZipFileContent";
    return false;
  }
  if (fileNameContentsMap.empty()) {
    LOG(ERROR) << "Empty fileNameContentsMap when ParseZipFileContent";
    return false;
  }
 
#if 0
  if (fileNameContentsMap.find("config.xml") ==
        fileNameContentsMap.end()) {
    LOG(ERROR) << "No config.xml file in this zipFileContent";
    return false;
  }
#endif
  if (fileNameContentsMap.find("config.xml") !=
        fileNameContentsMap.end()) {
    zipGifType_ = VIDEO_GIF;
  }
  if (fileNameContentsMap.find("ini") !=
        fileNameContentsMap.end()) {
    zipGifType_ = IMG_GIF;
  }

  if (VIDEO_GIF == zipGifType_) {
    VLOG(4) << "VIDEO_GIF";
    for (std::map<std::string, std::string>::iterator map_it
          = fileNameContentsMap.begin();
          map_it != fileNameContentsMap.end(); ++map_it) {     // the 'config.xml' file 
      if (map_it->first == "config.xml") {
        if (!ParseFramesHandledParam(map_it->second)) {
          LOG(ERROR) << "ParseFramesHandledParam error when ParseZipFileContent";
          return false;
        }
      }

      if (map_it->first.find("pathm") != std::string::npos) {  // a 'pathm*' file
        if (!AddPathmGPointsMap(map_it->first, map_it->second)) {
          LOG(ERROR) << "AddPathmGPointsMap error when ParseZipFileContent "
            << "for pathm = " << map_it->first;
          return false;
        }
      }
    }
  } else if (IMG_GIF == zipGifType_) {
    VLOG(4) << "IMG_GIF";
    /*
     * Parse the 'ini' file, which content such as
     * 0:2000:path:0.jpg;1:2000:path_1:1.jpg;
     */
    std::vector<std::string> imgSegs;
    SplitString(fileNameContentsMap["ini"], ';', &imgSegs);
    for (int i = 0; i < imgSegs.size(); i++) {
      /// A valid img frameInfo
      if (imgSegs[i].find(':') != std::string::npos) {
        std::vector<std::string> imgFrameSegs;
        SplitString(imgSegs[i], ':', &imgFrameSegs);
        // int frameIndex = StringToInt(imgFrameSegs[0]);  // frameIndex
        int frameDelay = StringToInt(imgFrameSegs[1]);
        std::string frameClipPathFileName = imgFrameSegs[2];  // (TODO) parse Path
        std::string frameFileName = imgFrameSegs[3];
        if (fileNameContentsMap.find(frameFileName)
              == fileNameContentsMap.end()) {  // No Exist this imgFrame
          LOG(ERROR) << "No Exist imgFrame: " << frameFileName
            << " when ParseZipFileContent for IMG_GIF with "
            << "ini-file content: " << fileNameContentsMap["ini"];
          return false;
        }
        /// Parse original frame image
        IMGGifFrame imgGifFrame;
        imgGifFrame.image_ = ImageIncludeAlpha(frameFileName,
              fileNameContentsMap[frameFileName]);
        imgGifFrame.delay_ = frameDelay;
        imgGifFrame.pathmName_ = frameClipPathFileName;
        imgGifFrames_.push_back(imgGifFrame);
        
        /// Parse this frame's clipPath orignal Points
        if (fileNameContentsMap.find(frameClipPathFileName)
              != fileNameContentsMap.end()) {  // Exist this imgFrame's clipPath file
          if (!AddPathmGPointsMap(frameClipPathFileName,
                  fileNameContentsMap[frameClipPathFileName])) {
            LOG(ERROR) << "AddPathmGPointsMap error when ParseZipFileContent "
              << "for pathm = " << frameClipPathFileName
              << " with zipGifType_ = IMG_GIF";
            return false;
          }
        }
      }
    }
  }
  return true;
}

bool ZipGif::HandleFramesByHandledParam() {
  if (VIDEO_GIF == zipGifType_) {
#if 0
    /// Handle in main-thread
    for (int index = 0;
          index < videoFramesInfo_.frameImgs_.size(); index++) {
      int frameIndex = 0;
      if (POSITIVE_ORDER == playOrder_) {
        frameIndex = index;
      } else {
        frameIndex = videoFramesInfo_.frameImgs_.size() - 1 - index;
      }
      ZipGifFrame zipGifFrame;
      if (frameHandledParams_.empty()) {
        zipGifFrame.frameImg_ = CvToMagick(
              videoFramesInfo_.frameImgs_[frameIndex]);
        zipGifFrame.delay_ = 1000 / videoFramesInfo_.fps_;
      } else {
        int handledParamIndex = index % frameHandledParams_.size();
        zipGifFrame.delay_ =
          frameHandledParams_[handledParamIndex].delay_;
        std::string pathmName =
          frameHandledParams_[handledParamIndex].pathmName_;
        int maskXoff = frameHandledParams_[handledParamIndex].maskXoff_;
        int maskYoff = frameHandledParams_[handledParamIndex].maskYoff_;
        Magick::Image baseImg;
        int maskId = frameHandledParams_[handledParamIndex].maskId_;
        if (maskId < maskImgInfos_.size()
              && maskImgInfos_[maskId].maskImg_.columns() > 0
              && maskImgInfos_[maskId].maskImg_.rows() > 0) {  // Add mask
          baseImg = Magick::Image(Magick::Geometry(
                  maskImgInfos_[maskId].maskImg_.columns(),
                  maskImgInfos_[maskId].maskImg_.rows()), "transparent");
          /// ImgClipPath
          Magick::Image img = CvToMagick(
            videoFramesInfo_.frameImgs_[frameIndex]);
          if (pathmNamePointsMap_.find(pathmName) != pathmNamePointsMap_.end()
                && pathmNamePointsMap_[pathmName].size() > 2) {
            ImgClipPath(pathmNamePointsMap_[pathmName], img);
          }
          baseImg.composite(img, maskXoff, maskYoff,
                Magick::OverCompositeOp);
          baseImg.composite(maskImgInfos_[maskId].maskImg_, 0, 0,
                Magick::OverCompositeOp);
        } else {    // No need add mask
          baseImg = CvToMagick(
            videoFramesInfo_.frameImgs_[frameIndex]);
          /// ImgClipPath
          if (pathmNamePointsMap_.find(pathmName) != pathmNamePointsMap_.end()
                && pathmNamePointsMap_[pathmName].size() > 2) {
            VLOG(4) << "ImgClipPath for " << index << "th frames";
            news::Timer clipPathTimer;
            clipPathTimer.Begin();
            ImgClipPath(pathmNamePointsMap_[pathmName], baseImg);
            clipPathTimer.End();
            VLOG(4) << "ImgClipPath handled for " << index << "th frames"
              << ", consume time: " << clipPathTimer.GetMs() << "ms";
          }
        }
        baseImg.backgroundColor("transparent");
        zipGifFrame.frameImg_ = baseImg;
        // baseImg.write("/tmp/clipPath/" + IntToString(index) + ".png");
      }
      handledZipGifFrames_.push_back(zipGifFrame);
    }
#endif
    
    /// Handle in multi-threads mode
    int handleThreadCnt = videoFramesInfo_.frameImgs_.size();
    base::Mutex frameImgsMutex;
    base::CondVar frameImgsHandleCondVar;
    shared_ptr<ZipGifFrameHandleThread> zipGifFrameHandleThreads[handleThreadCnt];
    for (int index = 0; index < handleThreadCnt; index++) {
      zipGifFrameHandleThreads[index] = shared_ptr<ZipGifFrameHandleThread>(
            new ZipGifFrameHandleThread(
            index, handleThreadCnt, this,
            &frameImgsMutex, &frameImgsHandleCondVar,
            &handledZipGifFramesIdMaps_));
      zipGifFrameHandleThreads[index]->Start();
    }

    frameImgsMutex.Lock();    // Lock
    while (handledZipGifFramesIdMaps_.size() < handleThreadCnt) {
      frameImgsHandleCondVar.Wait(&frameImgsMutex);
    }
    frameImgsMutex.Unlock();  // Unlock

    /// VideoFrameImgsDestroy
    videoFramesInfo_.VideoFrameImgsDestroy();
  } else if (IMG_GIF == zipGifType_) {
    /// Handle in multi-threads mode
    int handleThreadCnt = imgGifFrames_.size();
    base::Mutex frameImgsMutex;
    base::CondVar frameImgsHandleCondVar;
    shared_ptr<ZipGifFrameHandleThread> zipGifFrameHandleThreads[handleThreadCnt];
    for (int index = 0; index < handleThreadCnt; index++) {
      zipGifFrameHandleThreads[index] = shared_ptr<ZipGifFrameHandleThread>(
            new ZipGifFrameHandleThread(
            index, handleThreadCnt, this,
            &frameImgsMutex, &frameImgsHandleCondVar,
            &handledZipGifFramesIdMaps_));
      zipGifFrameHandleThreads[index]->Start();
    }
    
    frameImgsMutex.Lock();    // Lock
    while (handledZipGifFramesIdMaps_.size() < handleThreadCnt) {
      frameImgsHandleCondVar.Wait(&frameImgsMutex);
    }
    frameImgsMutex.Unlock();  // Unlock

    /// ****** Adjustment all the handled image frames set center
    std::map<int, ZipGifFrame>::iterator frame_it
      = handledZipGifFramesIdMaps_.begin();
    bool shouldAdjust = false;
    /// find out the maxWidth and maxHeight
    int maxWidth = frame_it->second.frameImg_.columns();
    int maxHeight = frame_it->second.frameImg_.rows();
    for (; frame_it != handledZipGifFramesIdMaps_.end();
          ++frame_it) {
      if (frame_it->second.frameImg_.columns() > maxWidth) {
        maxWidth = frame_it->second.frameImg_.columns();
        shouldAdjust = true;
      }
      if (frame_it->second.frameImg_.rows() > maxHeight) {
        maxHeight = frame_it->second.frameImg_.rows();
        shouldAdjust = true;
      }
      if (frame_it->second.frameImg_.columns() < maxWidth ||
            frame_it->second.frameImg_.rows() < maxHeight) {
        shouldAdjust = true;
      }
    }
    if (shouldAdjust) {
      VLOG(4) << "SHOULD-ADJUST";
      for (frame_it = handledZipGifFramesIdMaps_.begin();
            frame_it != handledZipGifFramesIdMaps_.end();
            ++frame_it) {
        Magick::Image canvas(
              Magick::Geometry(maxWidth, maxHeight),
              Magick::Color("transparent"));
        canvas.backgroundColor("transparent");
        int xOff
          = (maxWidth - frame_it->second.frameImg_.columns()) / 2;
        int yOff
          = (maxHeight - frame_it->second.frameImg_.rows()) / 2;
        canvas.composite(frame_it->second.frameImg_,
              xOff, yOff,
              Magick::OverCompositeOp);
        /// Instead the original frameImg with the canvas img
        frame_it->second.frameImg_ = canvas;
      }
    }

    /// imgGifFrames clear
    imgGifFrames_.clear();
  }
  return true;
}

bool ZipGif::ParseFramesHandledParam(
      const std::string& configXmlContent) {
  TiXmlDocument* xmlDocument = new TiXmlDocument();
  xmlDocument->Parse(configXmlContent.c_str());
  TiXmlPrinter printer;
  xmlDocument->Accept(&printer);
  VLOG(5) << "configXmlContent: " << printer.CStr();
  TiXmlElement* rootElement = xmlDocument->RootElement();
  VLOG(5) << "configXml rootName: " << rootElement->Value();
  TiXmlElement* childElement = rootElement->FirstChildElement();
  for (; childElement;
        childElement = childElement->NextSiblingElement()) {
    std::string nodeName = childElement->Value();
    VLOG(5) << "nodeName: " << nodeName;
    if ("OR" == nodeName) {
      const char* contentPtr = childElement->GetText();
      if (contentPtr) {
        std::string order = contentPtr;
        if ("0" == order) {
          playOrder_ = POSITIVE_ORDER;
        } else {
          playOrder_ = REVERSE_ORDER;
        }
        VLOG(5) << "playOrder_: " << playOrder_;
      }
    }
    if ("NT" == nodeName) {
      const char* contentPtr = childElement->GetText();
      if (contentPtr) {
        playTimes_ = StringToInt(std::string(contentPtr));
        VLOG(5) << "playTimes_: " << playTimes_;
      }
    }
    
    if ("MS" == nodeName) {
      TiXmlNode* pNode = NULL;
      for (pNode = childElement->FirstChildElement(); pNode;
            pNode = pNode->NextSiblingElement()) {
        if ("M" == std::string(pNode->Value())) {
          TiXmlElement* eChildElement = pNode->FirstChildElement();
          if (NULL == eChildElement) {
            continue;
          }
          for (; eChildElement;
                eChildElement = eChildElement->NextSiblingElement()) {
            std::string enodeName = eChildElement->Value();
            const char* valuePtr = eChildElement->GetText();
            if (!valuePtr) {
              continue;
            }
            if ("MP" == enodeName) {
              std::string maskUrl = valuePtr;
              if (maskUrl.find("http") != std::string::npos) {  // An web maskUrl
                MaskImgInfo maskImgInfo(maskUrl,
                      Magick::Image(maskUrl));
                maskImgInfos_.push_back(maskImgInfo);
              } else {    // A local maskImg relative path
                MaskImgInfo maskImgInfo(maskUrl,
                      Magick::Image(FLAGS_local_mask_imgs_dir + "/" + maskUrl));
                maskImgInfos_.push_back(maskImgInfo);
              }
            }
          }
        }
      }
    }
    if ("ES" == nodeName) {
      TiXmlNode* pNode = NULL;
      int i = 0;
      for (pNode = childElement->FirstChildElement(); pNode;
            pNode = pNode->NextSiblingElement()) {
        i++;
        if ("E" == std::string(pNode->Value())) {
          TiXmlElement* eChildElement = pNode->FirstChildElement();
          if (NULL == eChildElement) {
            continue;
          }
          FrameHandledParam frameHandledParam;
          for (; eChildElement;
                eChildElement = eChildElement->NextSiblingElement()) {
            std::string enodeName = eChildElement->Value();
            const char* valuePtr = eChildElement->GetText();
            if (!valuePtr) {
              continue;
            }
            if ("ID" == enodeName) {
              frameHandledParam.frameId_ = valuePtr;
            } else if ("IM" == enodeName) {
              frameHandledParam.frameName_ = valuePtr;
            } else if ("DE" == enodeName) {
              frameHandledParam.delay_ = StringToInt(std::string(valuePtr));
            } else if ("PA" == enodeName) {
              frameHandledParam.pathmName_ = valuePtr;
            } else if ("MF" == enodeName) {
              if (!ParsePointCoorRatio(std::string(valuePtr),
                        frameHandledParam.mtxFrom_)) {
                LOG(ERROR) << "ParsePointCoorRatio mtxFrom error for node " << i;
                return false;
              }
            } else if ("MT" == enodeName) {
              if (!ParsePointCoorRatio(std::string(valuePtr),
                        frameHandledParam.mtxTo_)) {
                LOG(ERROR) << "ParsePointCoorRatio mtxTo error for node " << i;
                return false;
              }
            } else if ("MK" == enodeName) {
              frameHandledParam.maskId_ = StringToInt(std::string(valuePtr));
            } else if ("MKX" == enodeName) {
              frameHandledParam.maskXoff_ = StringToInt(std::string(valuePtr));
            } else if ("MKY" == enodeName) {
              frameHandledParam.maskYoff_ = StringToInt(std::string(valuePtr));
            } else if ("AL" == enodeName) {
              frameHandledParam.alpha_ = StringToInt(std::string(valuePtr));
            }
          }
          if (frameHandledParam.mtxFrom_.size()
                != frameHandledParam.mtxTo_.size()) {
            LOG(ERROR) << i << "th frame don't have the same mtxFrom "
              << "and mtxTo with mtxFrom = " << frameHandledParam.mtxFrom_.size()
              << ", mtxTo = " << frameHandledParam.mtxTo_.size();
            continue;
          }
#if 0
          frameIdHandledParamMap_.insert(
                std::pair<std::string, FrameHandledParam>(
                  frameHandledParam.frameId_, frameHandledParam));
#endif
          frameHandledParams_.push_back(frameHandledParam);
        }
      }
    }
  }
#if 0
  /// TEST
  VLOG(4) << "***** FrameHandledParam number: "
    << frameIdHandledParamMap_.size();
  if (frameIdHandledParamMap_.find("20") !=
        frameIdHandledParamMap_.end()) {
    VLOG(4) << "mtxFrom.cnt = " << frameIdHandledParamMap_["20"].mtxFrom_.size()
      << ", mtxTo.cnt = " << frameIdHandledParamMap_["20"].mtxTo_.size();
  }
#endif

  VLOG(4) << "***** FrameHandledParam number: "
    << frameHandledParams_.size();
  if (frameHandledParams_.size() >= 21) {
    VLOG(4) << "mtxFrom.cnt = " << frameHandledParams_[20].mtxFrom_.size()
      << ", mtxTo.cnt = " << frameHandledParams_[20].mtxTo_.size();
  }
  return true;
}
  
bool ZipGif::ParsePointCoorRatio(const std::string mtxStr,
      std::vector<PointCoorRatio>& pointCoorRatios) {
  std::vector<std::string> coors;
  SplitString(mtxStr, ',', &coors);
  if (coors.size() % 2 != 0) {  // not a Even count
    LOG(ERROR) << mtxStr << " don't contain Even number";
    return false;
  }
  for (int i = 0; i < coors.size(); i+=2) {
    PointCoorRatio pointCoorRatio;
    pointCoorRatio.xR_ = StringToDouble(coors[i]);
    pointCoorRatio.yR_ = StringToDouble(coors[i + 1]);
    pointCoorRatios.push_back(pointCoorRatio);
  }
  return true;
}
  
bool ZipGif::AddPathmGPointsMap(const std::string& pathmName,
      const std::string& pathmContent) {
  if (pathmName.empty()) {
    LOG(ERROR) << "Empty pathmName when AddPathmGPointsMap";
    return false;
  }
  /// (TODO) some kind "pathmName" file may be content, should
  /// add std::pair("pathmName", gPoints), where gPoints is empty. 
  if (pathmContent.empty()) {
    LOG(ERROR) << "Empty pathmContent when AddPathmGPointsMap";
    return false;
  }
  std::vector<std::string> pointPairStrs;
  SplitString(pathmContent, ';', &pointPairStrs);
  for (int i = 0; i < pointPairStrs.size(); i++) {
    VLOG(5) << i << "th PointPairStr = " << pointPairStrs[i];
    std::vector<std::string> posStrs;
    SplitString(pointPairStrs[i], ':', &posStrs);
    if (posStrs.size() != 2) {
      continue;
    }
    if (!IsNumberStr(posStrs[0]) || !IsNumberStr(posStrs[1])) {
      continue;
    }
    Point point(StringToDouble(posStrs[0]),  // x
            StringToDouble(posStrs[1]));     // y
    pathmNamePointsMap_[pathmName].push_back(point);
  }
  /// for Test
  VLOG(4) << "pathmName = " << pathmName
    << ", pathmContent = " << pathmContent;
  for (std::vector<Point>::iterator it = pathmNamePointsMap_[pathmName].begin();
        it != pathmNamePointsMap_[pathmName].end(); ++it) {
    VLOG(5) << "Point.x = " << it->x << ", Point.y = " << it->y;
  }
  return true;
}

/// for ZipGifFrameHandleThread
ZipGifFrameHandleThread::ZipGifFrameHandleThread(
      const int& imgId, const int& totalImgsCnt,
      ZipGif* zipGif,
      base::Mutex* imgsMutex,
      base::CondVar* imgsHandledCondVar,
      std::map<int, ZipGifFrame>* handledZipGifFramesIdMaps) {
  imgId_ = imgId;
  totalImgsCnt_ = totalImgsCnt;
  zipGif_ = zipGif;
  imgsMutex_ = imgsMutex;
  imgsHandledCondVar_ = imgsHandledCondVar;
  handledZipGifFramesIdMaps_ = handledZipGifFramesIdMaps;
}

ZipGifFrameHandleThread::~ZipGifFrameHandleThread() {
}

void ZipGifFrameHandleThread::Run() {
  if (VIDEO_GIF == zipGif_->get_zipgif_type()) {
    int frameIndex = 0;
    VideoFramesInfo videoFramesInfo
      = zipGif_->get_video_frames_info();
    if (POSITIVE_ORDER == zipGif_->get_play_order()) {
      frameIndex = imgId_;
    } else {
      frameIndex = videoFramesInfo.frameImgs_.size() - 1 - imgId_;
    }

    std::vector<FrameHandledParam> frameHandledParams
      = zipGif_->get_frame_handled_params();

    ZipGifFrame zipGifFrame;
    if (frameHandledParams.empty()) {
      zipGifFrame.frameImg_ = CvToMagick(
            videoFramesInfo.frameImgs_[frameIndex]);
      zipGifFrame.delay_ = 1000 / videoFramesInfo.fps_;
    } else {
      std::vector<MaskImgInfo> maskImgInfos
        = zipGif_->get_mask_img_infos();

      std::map<std::string, std::vector<Point> > pathmNamePointsMap
        = zipGif_->get_pathm_name_points_map();

      int handledParamIndex = imgId_ % frameHandledParams.size();
      zipGifFrame.delay_ =
        frameHandledParams[handledParamIndex].delay_;
      std::string pathmName =
        frameHandledParams[handledParamIndex].pathmName_;
      int maskXoff = frameHandledParams[handledParamIndex].maskXoff_;
      int maskYoff = frameHandledParams[handledParamIndex].maskYoff_;
      Magick::Image baseImg;
      int maskId = frameHandledParams[handledParamIndex].maskId_;
      if (maskId < maskImgInfos.size()
            && maskImgInfos[maskId].maskImg_.columns() > 0
            && maskImgInfos[maskId].maskImg_.rows() > 0) {  // Add mask
        baseImg = Magick::Image(Magick::Geometry(
                maskImgInfos[maskId].maskImg_.columns(),
                maskImgInfos[maskId].maskImg_.rows()), "transparent");
        /// ImgClipPath
        Magick::Image img = CvToMagick(
              videoFramesInfo.frameImgs_[frameIndex]);
        if (pathmNamePointsMap.find(pathmName) != pathmNamePointsMap.end()
              && pathmNamePointsMap[pathmName].size() > 2) {
          ImgClipPath(pathmNamePointsMap[pathmName], img);
        }
        baseImg.composite(img, maskXoff, maskYoff,
              Magick::OverCompositeOp);
        baseImg.composite(maskImgInfos[maskId].maskImg_, 0, 0,
              Magick::OverCompositeOp);
      } else {  // No need add mask
        baseImg = CvToMagick(
              videoFramesInfo.frameImgs_[frameIndex]);
        /// ImgClipPath
        if (pathmNamePointsMap.find(pathmName) != pathmNamePointsMap.end()
              && pathmNamePointsMap[pathmName].size() > 2) {
          VLOG(4) << "ImgClipPath in thread for " << imgId_ << "th frames";
          news::Timer clipPathTimer;
          clipPathTimer.Begin();
          //std::vector<Point> pathmNamePoints = pathmNamePointsMap[pathmName];
          //ImgClipPath(pathmNamePoints, baseImg);
          ImgClipPath(pathmNamePointsMap[pathmName], baseImg);
          clipPathTimer.End();
          VLOG(4) << "ImgClipPath handled in thread for " << imgId_ << "th frames"
            << ", consume time: " << clipPathTimer.GetMs() << "ms";
        }
      }
      baseImg.backgroundColor("transparent");
      zipGifFrame.frameImg_ = baseImg;
    }

    handledZipGifFramesIdMaps_->insert(
          std::pair<int, ZipGifFrame>(imgId_,
            zipGifFrame));
  } else if (IMG_GIF == zipGif_->get_zipgif_type()) {
    int frameIndex = imgId_;
    ZipGifFrame zipGifFrame;
    Magick::Image baseImg
      = zipGif_->get_imggif_frames()[frameIndex].image_;
    zipGifFrame.delay_
      = zipGif_->get_imggif_frames()[frameIndex].delay_;
    std::string pathmName
      = zipGif_->get_imggif_frames()[frameIndex].pathmName_;
    /// ClipPath
    std::map<std::string, std::vector<Point> > pathmNamePointsMap
      = zipGif_->get_pathm_name_points_map();
    if (pathmNamePointsMap.find(pathmName) != pathmNamePointsMap.end()
          && pathmNamePointsMap[pathmName].size() > 2) {
      VLOG(4) << "ImgClipPath(IMG_GIF) in thread for "
        << imgId_ << "th frames";
      news::Timer clipPathTimer;
      clipPathTimer.Begin();
      ImgClipPath(pathmNamePointsMap[pathmName], baseImg);
      clipPathTimer.End();
      VLOG(4) << "ImgClipPath(IMG_GIF) handled in thread for "
        << imgId_ << "th frames"
        << ", consume time: " << clipPathTimer.GetMs() << "ms";
    }
    //baseImg.backgroundColor("transparent");
    zipGifFrame.frameImg_ = baseImg;
    
    handledZipGifFramesIdMaps_->insert(
          std::pair<int, ZipGifFrame>(imgId_,
            zipGifFrame));
  }

 
  imgsMutex_->Lock();    // Lock
  if (totalImgsCnt_ == handledZipGifFramesIdMaps_->size()) {
    imgsHandledCondVar_->Signal();
    VLOG(4) << "[TEST] " << imgId_ << "th thread is the last handled one";
  }
  imgsMutex_->Unlock();  // Unlock
}

bool IsNumberStr(const std::string& str) {
  if (str.empty()) {
    return false;
  }
  for (int i = 0; i < str[i]; i++) {
    if (str[i] < '0' || str[i] > '9') {
      if (str[i] != '.') {
        return false;
      }
    }
  }
  return true;
}
}  // namespace multimedia
}  // namesapce juyou
