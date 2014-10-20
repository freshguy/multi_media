// Copyright (c) 2013 Juyou Inc. All rights reserved.
// Author: wangjuntao@juyou.com (Juntao Wang)

#ifndef JUYOU_MULTI_MEDIA_PUBLIC_ELEMENT_CONTENT_H_
#define JUYOU_MULTI_MEDIA_PUBLIC_ELEMENT_CONTENT_H_

#include <string>
#include <vector>
#include "base/logging.h"
#include "base/flags.h"
#include "base/string_util.h"
#include "base/utf_string_conversions.h"
#include "base/utf_string_conversion_utils.h"
#include "file/file.h"
#include "juyou/multi_media/gif/gif_decoder.h"
#include "juyou/circles/public/utility.h"
#include "juyou/multi_media/public/zip_gif.h"
#include "juyou/multi_media/public/img_handler.h"

#define VIDEO_DECODE_DIR ("/tmp")

namespace juyou {
namespace multimedia {

/// Abstracte Element content class
class ElementContent {
 public:
  ElementContent() {
  }
  virtual ~ElementContent() {
  }
  virtual bool DecodeElementContent(
        const std::string& elementContent,
        const std::string& elementUrlName) = 0;
 
  /// Not all sub-class override this method
  virtual Magick::Image GetContentByDatumTime(
        const int& datumTime) {
    Magick::Image img;
    return img;
  }

  /// 
  virtual std::string GetElementContent() {
    return std::string("");
  }
};

/// Image element content
class ImgElementContent : public ElementContent {
 public:
  ImgElementContent() {
  }
  
  virtual ~ImgElementContent() {
  }
 
  virtual bool DecodeElementContent(
        const std::string& elementContent,
        const std::string& elementUrlName) {
    if (elementContent.empty()) {
      LOG(ERROR) << "Empty image content";
      return false;
    }
    imgContent_ = elementContent;
    return true;
  }

  virtual Magick::Image GetContentByDatumTime(
        const int& datumTime) {
    return ImgContent2Image(imgContent_);
  }

 private:
  std::string imgContent_;
};

/// Audio element content
class AudioElementContent : public ElementContent {
 public:
  AudioElementContent() {
  }

  virtual ~AudioElementContent() {
  }

  virtual bool DecodeElementContent(
        const std::string& elementContent,
        const std::string& elementUrlName) {
    if (elementContent.empty()) {
      LOG(ERROR) << "Empty audio content";
      return false;
    }
    audioContent_ = elementContent;
    return true;
  }

#if 0
  std::string GetAudioContentByDatumTime(
        const int& datumTime) {
    return audioContent_;
  }
#endif
  
  std::string GetElementContent() {
    return audioContent_;
  }

 private:
  std::string audioContent_;
};

/// Gif element content
class GifElementContent : public ElementContent {
 public:
  GifElementContent() : ElementContent() {
    gifDecoder_ = NULL;
  }
  
  virtual ~GifElementContent() {
    if (gifDecoder_) {
      delete gifDecoder_;
    }
    gifDecoder_ = NULL;
  }
  
  virtual bool DecodeElementContent(
        const std::string& elementContent,
        const std::string& elementUrlName) {
    if (elementContent.empty()) {
      LOG(ERROR) << "Empty gif content";
      return false;
    }
    gifDecoder_ = new GifDecoder(elementContent);
    if (!gifDecoder_->IsDecodeDone()) {
      LOG(ERROR) << "GifDecoder error";
      return false;
    }
    return true;
  }
  
  virtual Magick::Image GetContentByDatumTime(
        const int& datumTime) {
    int gifFrameCnt = gifDecoder_->GifFrameCount();
    int framesTimeSum = 0;
    int iCnt = 0;
    int frameIndex = 0;
    while (1) {
      frameIndex = iCnt % gifFrameCnt;
      framesTimeSum += gifDecoder_->GifFrameDelayTime(
            frameIndex);
      if (datumTime <= framesTimeSum) {
        break;
      }
      iCnt++;
    }
    return gifDecoder_->GifFrameImage(frameIndex);
  }

 private:
  GifDecoder* gifDecoder_;
};

/// ZipGif element content
class ZipGifElementContent : public ElementContent {
 public:
  ZipGifElementContent() : ElementContent() {
  }

  virtual ~ZipGifElementContent() {
    if (zipGif_) {
      delete zipGif_;
    }
    zipGif_ = NULL;
  }

  virtual bool DecodeElementContent(
        const std::string& elementContent,
        const std::string& elementUrlName) {
    if (elementContent.empty()) {
      LOG(ERROR) << "Empty ZipGif content";
      return false;
    }
    zipGif_ = new ZipGif;
    if (!zipGif_->ParseZipFileContent(elementContent)) {
      LOG(ERROR) << "ParseZipFileContent when DecodeElementContent";
      return false;
    }
    if (!zipGif_->HandleFramesByHandledParam()) {
      LOG(ERROR) << "HandleFramesByHandledParam when DecodeElementContent";
      return false;
    }
    return true;
  }
  
  virtual Magick::Image GetContentByDatumTime(
        const int& datumTime) {
    int zipGifFrameCnt = zipGif_->GetZipGifFrameCount();
    int framesTimeSum = 0;
    int iCnt = 0;
    int frameIndex = 0;
    while (1) {
      frameIndex = iCnt % zipGifFrameCnt;
      framesTimeSum += zipGif_->GetZipGifFrameDelay(frameIndex);
      if (datumTime <= framesTimeSum) {
        break;
      }
      iCnt++;
    }
    return zipGif_->GetZipGifFrameImage(frameIndex);
  }

 private:
  ZipGif* zipGif_;
};

/// BackGround element content
class BackGroundImgElementContent : public ElementContent {
 public:
  BackGroundImgElementContent() {
  }
   
  virtual ~BackGroundImgElementContent() {
  }
  
  virtual bool DecodeElementContent(
       const std::string& elementContent,
       const std::string& elementUrlName) {
    if (elementContent.empty()) {
      LOG(ERROR) << "Empty BackGround image content";
      return false;
    }
    backGroundImgContent_ = elementContent;
    return true;
  };
  
  virtual Magick::Image GetContentByDatumTime(
        const int& datumTime) {
    return ImgContent2Image(backGroundImgContent_);
  }

 public:
  std::string backGroundImgContent_;
};

struct VideoInternalAudioInfo {
  std::string audioUrlName_;  // such as: 2355u8coioiuffjdflujloudqu23.wav
  std::string audioContent_;
  //MessageElement* messageElement_;
};

/// Video element content
class VideoElementContent : public ElementContent {
 public:
  VideoElementContent() {  // }MessageElement* messageElement) {
#if 0
    messageElement_ = messageElement;
    MsgActionPlay* msgActionPlay = NULL;
    for (std::vector<MsgAction*>::iterator
          it = messageElement_->actions_->begin();
          it != messageElement_->actions_->end(); ++it) {
    }
    playStartTime_ = 0;
    playDuration_ = 0;
#endif
  }

  VideoElementContent(int playStartTime, int playDuration) {
    playStartTime_ = playStartTime;
    playDuration_ = playDuration;
  }
  
  virtual ~VideoElementContent() {
    videoFramesInfo_.VideoFrameImgsDestroy();
  }
  
  virtual bool DecodeElementContent(
        const std::string& elementContent,
        const std::string& elementUrlName) {
    if (elementContent.empty()) {
      LOG(ERROR) << "Empty elementContent for "
        << "VideoElementContent";
      return false;
    }
    if (elementUrlName.empty()) {
      LOG(ERROR) << "Empty elementUrlName for "
        << "VideoElementContent";
      return false;
    }
    
    std::string videoDiskFileName = VIDEO_DECODE_DIR;
    videoDiskFileName += "/";
    videoDiskFileName += Utility::GetUuid("VIDEO_");
    videoDiskFileName += elementUrlName;
    
    if (!file::File::WriteStringToFile(elementContent,
            videoDiskFileName)) {
      LOG(ERROR) << "Write video content to disk error for "
        << videoDiskFileName;
      return false;
    }
    /// Get the video's Frames
    if (!Video2FrameImagesFO(videoDiskFileName,
            videoFramesInfo_)) {
      LOG(ERROR) << "Video2FramesImagesFO error when DecodeElementContent "
        << "for ElementType::VIDEO";
      file::File::DeleteRecursively(videoDiskFileName);
      return false;
    }

    /// Extract audio from video
    std::vector<std::string> elementUrlNameSegs;
    SplitString(elementUrlName, '.', &elementUrlNameSegs);
    std::string extractedAudioFileName = VIDEO_DECODE_DIR;
    extractedAudioFileName += "/";
    extractedAudioFileName += Utility::GetUuid("");
    extractedAudioFileName += "_";
    extractedAudioFileName += elementUrlNameSegs[0];
    extractedAudioFileName += ".wav";

    std::string extractAudioCommand = "ffmpeg -i ";
    extractAudioCommand += videoDiskFileName;
    extractAudioCommand += " -f wav -vn -y ";
    extractAudioCommand += extractedAudioFileName;
    if (system(extractAudioCommand.c_str()) == -1) {
      LOG(ERROR) << "Extract audio from video error for "
        << elementUrlName;
      file::File::DeleteRecursively(videoDiskFileName);
      file::File::DeleteRecursively(extractedAudioFileName);
      return false;
    }
   
    videoInternalAudioInfo_.audioUrlName_ = elementUrlNameSegs[0] + ".wav";
    if (!file::File::ReadFileToString(extractedAudioFileName,
            &videoInternalAudioInfo_.audioContent_)) {
      LOG(ERROR) << "Read VideoInternal audio error for "
        << elementUrlName;
      file::File::DeleteRecursively(videoDiskFileName);
      file::File::DeleteRecursively(extractedAudioFileName);
      return false;
    }

    // videoInternalAudioInfo_.messageElement_ = messageElement_;
    return true;
  }

  virtual Magick::Image GetContentByDatumTime(
        const int& datumTime) {
    if (datumTime < playStartTime_
          || datumTime > playStartTime_ + playDuration_) {
      return Magick::Image(Magick::Geometry(1, 1),
            Magick::Color("transparent"));
    }
    int totalFrameCnt = videoFramesInfo_.frameCnt_;
    int frameInterval = 1000 / videoFramesInfo_.fps_;  // ms
    int totalTimeSum = playStartTime_;
    int frameIndex = 0;
    while (true) {
      totalTimeSum += frameInterval;
      if (datumTime <= totalTimeSum || frameIndex >= totalFrameCnt - 1) {
        break;
      }
      frameIndex++;
    }
    VLOG(4) << "datumTime: " << datumTime
      << ", totalFrameCnt: " << videoFramesInfo_.frameCnt_
      << ", frameImgs.size: " << videoFramesInfo_.frameImgs_.size()
      << ", frameInterval: " << frameInterval
      << ", frameIndex: " << frameIndex;
    return CvToMagick(
          videoFramesInfo_.frameImgs_[frameIndex]);
  }

 public:
  VideoFramesInfo videoFramesInfo_;
  VideoInternalAudioInfo videoInternalAudioInfo_;
  /// The MessageElement this VideoElementContent belongs to
  //MessageElement* messageElement_;
  int playStartTime_;
  int playDuration_;
};

/// ****** TEXT element content ******
class TextElementContent : public ElementContent {
 public:
  TextElementContent(int playStartTime, int playDuration,
        int regionWidth, int regionHeight,
        int fontSize, int fontColor,
        int fontDirection, const std::string& fontName) {
    playStartTime_ = playStartTime;
    playDuration_ = playDuration;
    regionWidth_ = regionWidth;
    regionHeight_ = regionHeight;
    /// Tune the fontSize_
    if (fontSize <= 20) {
      fontSize_ = fontSize - 15;
    } else {
      fontSize_ = fontSize - 20;
    }
    fontColor_ = fontColor;
    fontDirection_ = fontDirection;
    // fontName_ = "helvetica";
    fontName_ = fontName.substr(fontName.rfind('/') + 1);
  }

  virtual ~TextElementContent() {
  }
  
  virtual bool DecodeElementContent(
        const std::string& elementContent,
        const std::string& elementUrlName) {
    /// string convert to wstring
    text_ = UTF8ToWide(base::StringPiece(elementContent));

    /// Static the actual characterCnt
    actualCharacterCnt_ = text_.length() + 1;
    return true;
  }

  virtual Magick::Image GetContentByDatumTime(
        const int& datumTime) {
    Magick::Image image(Magick::Geometry(regionWidth_, regionHeight_),
          Magick::Color("transparent"));
    if (datumTime < playStartTime_) {
      return image;
    }
    /// Set the font attribute
    image.font(FLAGS_fonts_install_dir + "/" + fontName_);       // fontName
    image.fontPointsize(fontSize_);                              // fontSize
    // svgColor: such as "#ffffffff";
    std::string svgColor = "#" + StringPrintf("%.x", fontColor_);
    image.fillColor(Magick::Color(svgColor));                    // fontColor
    // image.strokeColor("white");                               // font-border color
    // image.strokeWidth(3);
    image.textEncoding("UTF-8");
    int charTimeSlice = playDuration_ / actualCharacterCnt_;
    if (datumTime >= playStartTime_
          && datumTime <= playStartTime_ + playDuration_) {  // in duration
      /// Calculate the end-char index
      int totalTimeSum = playStartTime_;
      int endCharIndex = 0;
      while (true) {
        totalTimeSum += charTimeSlice;
        if (datumTime <= totalTimeSum || endCharIndex >= text_.length()) {
          break;
        }
        endCharIndex++;
      }
      /// Draw characters until the end-char index
      AnnotateImage(endCharIndex, image);
    } else {  // exceed duration, draw all characters
      int endCharIndex = text_.length() - 1;
      AnnotateImage(endCharIndex, image);
    }
    return image;
  }

 private:
  void AnnotateImage(const int& endCharIndex,
        Magick::Image& image) {
    if (1 == fontDirection_) {  // vertical
      int currentXoff = regionWidth_ - fontSize_;
      int currentYoff = 0;
      for (int i = 0; i <= endCharIndex; i++) {
        if (text_[i] == '\n') {
          currentXoff -= fontSize_;
          currentYoff = 0;
        } else {
          std::string currentCharStr;
          base::WriteUnicodeCharacter(text_[i], &currentCharStr);
          image.annotate(currentCharStr,
                Magick::Geometry(fontSize_, fontSize_,
                  currentXoff, currentYoff));
          currentYoff += fontSize_;
        }
      }
    } else {
      int currentXoff = 0;
      int currentYoff = 0;
      for (int i = 0; i <= endCharIndex; i++) {
        if (text_[i] == '\n') {
          currentXoff = 0;
          currentYoff += fontSize_;
        } else {
          std::string currentCharStr;
          base::WriteUnicodeCharacter(text_[i], &currentCharStr);
          image.annotate(currentCharStr,
                Magick::Geometry(fontSize_, fontSize_,
                  currentXoff, currentYoff));
          currentXoff += fontSize_;
        }
      }
    }
  }

 public:
  int playStartTime_;
  int playDuration_;
  int regionWidth_;
  int regionHeight_;
  
  std::wstring text_;
  int actualCharacterCnt_;
  int fontSize_;
  int fontColor_;
  int fontDirection_;
  std::string fontName_;
};
}  // namespace multimedia
}  // namespace juyou
#endif  // JUYOU_MULTI_MEDIA_PUBLIC_ELEMENT_CONTENT_H_
