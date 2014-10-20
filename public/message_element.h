// Copyright (c) 2013 Juyou Inc. All rights reserved.
// Author: wangjuntao@juyou.com (Juntao Wang)

#ifndef JUYOU_MULTI_MEDIA_PUBLIC_MESSAGE_ELEMENT_H_
#define JUYOU_MULTI_MEDIA_PUBLIC_MESSAGE_ELEMENT_H_

#include <string>
#include <vector>
#include "base/logging.h"
#include "base/flags.h"
#include "juyou/third_party/CImg/CImg.h"
#include "juyou/multi_media/public/element_content.h"
#include "juyou/multi_media/msg_action/msg_action.h"
#include "juyou/multi_media/msg_action/msg_action_play.h"
#include "juyou/multi_media/msg_action/msg_action_text_animate.h"
#include "juyou/multi_media/public/img_handler.h"

using namespace cimg_library;  // NOLINT

#define SAMPLE_HANDLE_INTERVAL 20  // in ms

#define BIG_DOUBLE 10000.0
#define BIG_INT 32767

namespace juyou {
namespace multimedia {

struct RemoteFileInfo {
  std::string typeName;
  std::string origLocalFileName;
  std::string remoteFileName;
  std::string remoteFileContent;
};

enum ElementType {
  UNKNOWN = 0,
  AUDIO_MEDIAPLAYER = 1,
  GIF = 2,
  ZIP_GIF = 3,
  IMG = 4,
  TEXT = 5,
  VIDEO = 6,
  BACKGROUND_IMG = 7,
};

template<class T>
class ElementHandledInfo;

class MessageElement {
 public:
  explicit MessageElement(int screenWidth, int screenHeight) {
    screenWidth_ = screenWidth;
    screenHeight_ = screenHeight;
    elementType_ = UNKNOWN;
    elementContent_ = NULL;
    isReturn_ = false;
    visible_ = true;
    scale_ = BIG_DOUBLE;
    degree_ = BIG_DOUBLE;
    x_ = BIG_INT;
    y_ = BIG_INT;
    w_ = BIG_INT;
    h_ = BIG_INT;
  }


  virtual ~MessageElement() {
    /// delete the elementContent_
    if (elementContent_) {
      delete elementContent_;
    }
    elementContent_ =NULL;
    /// delete the element's actions_
    for (std::vector<MsgAction*>::iterator it = actions_.begin();
          it != actions_.end(); ++it) {
      if (*it) {
        delete *it;
      }
      *it = NULL;
    }
    actions_.clear();
    VLOG(4) << "$$$$$ MessageElement Destroy";
  }

  bool SetElementContent(const std::string& elementContent,
        const std::string& elementUrlName) {
    switch (elementType_) {
      case AUDIO_MEDIAPLAYER: {
        AudioElementContent* elePtr = new AudioElementContent;
        if (!elePtr->DecodeElementContent(elementContent,
                elementUrlName)) {
          LOG(ERROR) << "SetElementContent error for AUDIO_MEDIAPLAYER";
          return false;
        }
        elementContent_ = elePtr;
        break;
      }
      case GIF: {
        GifElementContent* elePtr = new GifElementContent;
        if (!elePtr->DecodeElementContent(elementContent,
                elementUrlName)) {
          LOG(ERROR) << "SetElementContent error for GIF";
          return false;
        }
        elementContent_ = elePtr;
        break;
      }
      case ZIP_GIF: {
        ZipGifElementContent* elePtr = new ZipGifElementContent;
        if (!elePtr->DecodeElementContent(elementContent,
                elementUrlName)) {
          LOG(ERROR) << "SetElementContent error for ZIP_GIF";
          return false;
        }
        elementContent_ = elePtr;
        break;
      }
      case IMG: {
        ImgElementContent* elePtr = new ImgElementContent;
        if (!elePtr->DecodeElementContent(elementContent,
                elementUrlName)) {
          LOG(ERROR) << "SetElementContent error for IMG";
          return false;
        }
        elementContent_ = elePtr;
        break;
      }
      case VIDEO: {
        MsgActionPlay* msgActionPlay = NULL;
        for (std::vector<MsgAction*>::iterator it = actions_.begin();
              it != actions_.end(); ++it) {
          if (PLAY == (*it)->actionType_) {
            msgActionPlay = (MsgActionPlay*)*it;
            break;
          }
        }
        if (!msgActionPlay) {
          LOG(ERROR) << "No PLAY MsgAction when SetElementContent "
            << "for VIDEO element, elementUrlName = " << elementUrlName;
          return false;
        }

        int playStartTime = 0, playDuration = 0;
        playStartTime = msgActionPlay->startTime_;
        playDuration = msgActionPlay->duration_;
        VLOG(4) << "***** playStartTime = " << playStartTime
          << ", playDuration = " << playDuration
          << ". for elementUrlName = " << elementUrlName;

        VideoElementContent* elePtr = new VideoElementContent(
              playStartTime, playDuration);
        if (!elePtr->DecodeElementContent(elementContent,
                elementUrlName)) {
          LOG(ERROR) << "SetElementContent error for IMG";
          return false;
        }
        elementContent_ = elePtr;
        break;
      }
      case TEXT: {
        MsgActionTextAnimate* msgActionTextAnimate = NULL;
        for (std::vector<MsgAction*>::iterator it = actions_.begin();
              it != actions_.end(); ++it) {
          if (TEXT_ANIMATE == (*it)->actionType_) {
            msgActionTextAnimate = (MsgActionTextAnimate*)*it;
            break;
          }
        }
        if (!msgActionTextAnimate) {
          LOG(ERROR) << "No TEXT_ANIMATE MsgAction when SetElementContent "
            << "for TEXT element, elementUrlName = " << elementUrlName;
          return false;
        }
        int playStartTime = 0, playDuration = 0;
        playStartTime = msgActionTextAnimate->startTime_;
        playDuration = msgActionTextAnimate->duration_;
        VLOG(4) << "****** TEXT playStartTime = " << playStartTime
          << ", playDuration = " << playDuration
          << ". for elementUrlName = " << elementUrlName;
        
        TextElementContent* elePtr = new TextElementContent(
              playStartTime, playDuration, w_, h_,
              size_, color_, direction_, tf_);
        if (!elePtr->DecodeElementContent(elementContent,
                elementUrlName)) {
          LOG(ERROR) << "SetElementContent error for TEXT";
          return false;
        }
        elementContent_ = elePtr;
        break;
      }
      case BACKGROUND_IMG: {
        BackGroundImgElementContent* elePtr
          = new BackGroundImgElementContent;
        if (!elePtr->DecodeElementContent(elementContent,
                elementUrlName)) {
          LOG(ERROR) << "SetElementContent error for BACKGROUND_IMG";
          return false;
        }
        elementContent_ = elePtr;
        break;
      }
      case UNKNOWN: {
        LOG(ERROR) << "No Set elementType_ when SetElementContent";
        return false;
      }
      default: {
        LOG(ERROR) << "SetElementContent error for default switch";
        return false;
      }
    }
    return true;
  }

  bool HandleElementByActions(const int& datumTime,
        ElementHandledInfo<unsigned char>& elementHandledInfo);

 private:
  MsgAction* GetActionByDatumTime(const int& datumTime,
        int& statusDataIndex, bool& isVisible);

  /// for IMG, GIF, ZIP_GIF(Except video, audio etc)
  Magick::Image GetInitialImageByDatumTime(
        Magick::Image* datumTimeImage);

  void GetInitialImageByDatumTime(
      Magick::Image* datumTimeImage,
      ElementHandledInfo<unsigned char>& elementHandledInfo);

  /// for IMG, GIF, ZIP_GIF, BACKGROUND_IMG(Except video, audio etc)
  bool HandleDatumTimeElement(const int& datumTime,
        ElementHandledInfo<unsigned char>& elementHandledInfo);

 private:
  void RotateByDatumTimeContent( 
      Magick::Image* datumTimeImage,
      const double& degree,
      ElementHandledInfo<unsigned char>& elementHandledInfo);

  void ScaleByDatumTimeContent(
      Magick::Image* datumTimeImage,
      const double& scale,
      ElementHandledInfo<unsigned char>& elementHandledInfo);
  
  void TranslateByDatumTimeContent( 
      Magick::Image* datumTimeImage,
      const int& xOff, const int& yOff,
      ElementHandledInfo<unsigned char>& elementHandledInfo);

 public:
  int screenWidth_;
  int screenHeight_;
  std::string id_;
  ElementType elementType_;
  ElementContent* elementContent_;
  std::string url_;
  bool isReturn_;
  int x_;
  int y_;
  int w_;
  int h_;
  double scale_;
  double degree_;
  std::vector<MsgAction*> actions_;
  bool visible_;
  /// additional
  int volume_;        // specify for VIDEO, AUDIO_MEDIAPLAYER: volume
  std::string text_;  // specify for TEXT: content
  int size_;          // specify for TEXT: font size
  int textScreenW_;   // specify for TEXT: text show screen width
  int textScreenH_;   // specify for TEXT: text show screen height
  int color_;         // specify for TEXT: text color
  std::string tf_;    // specify for TEXT: text font
  int direction_;     // specify for TEXT: characters direction(0 - lateral, 1 - vertical)
  int duration_;      // specify for TEXT: text show duration
};

enum ElementHandledStatus {
  HANDLING = 0,
  FRAME_DONE = 1,
  TOTALLY_DONE = 2,
};

template<class T>
class ElementHandledInfo {
 public:
  ElementHandledInfo() {
    handledStatus_ = HANDLING;
    seqId_ = 0;
    finalTimeStamp_ = 0;
    isVisible_ = true;
    xOff_ = 0;
    yOff_ = 0;
  }

 public:
  ElementHandledStatus handledStatus_;
  int seqId_;               // The thread-id which the sample frame comes from 
  // CImg<T> handledImg_;      // Represent the handled Bitmap
  Magick::Image handledImg_;  // Represent the handled Bitmap
  int finalTimeStamp_;  // The sample frame final timeStamp
  bool isVisible_;
  int xOff_;
  int yOff_;
};
}  // namespace multimedia
}  // namespace juyou
#endif  // JUYOU_MULTI_MEDIA_PUBLIC_MESSAGE_ELEMENT_H_
