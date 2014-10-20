// Copyright (c) 2013 Juyou Inc. All rights reserved.
// Author: wangjuntao@juyou.com (Juntao Wang)

#include "base/time.h"
#include "juyou/multi_media/public/message_element.h"
#include "juyou/multi_media/msg_action/msg_action_rotate.h"
#include "juyou/multi_media/msg_action/msg_action_scale.h"
#include "juyou/multi_media/msg_action/msg_action_translate.h"
#include "juyou/multi_media/msg_action/msg_action_visibility.h"

namespace juyou {
namespace multimedia {

bool MessageElement::HandleDatumTimeElement(const int& datumTime,
      ElementHandledInfo<unsigned char>& elementHandledInfo) {
  /// Get the element's datum-time action data
  int elementStatusDataIndex = 0;
  bool isElementVisible = true;
  MsgAction* elementDatumTimeAction = GetActionByDatumTime(
        datumTime, elementStatusDataIndex, isElementVisible);

  /// Get the element's datum-time content
  Magick::Image elementDatumTimeContent
    = elementContent_->GetContentByDatumTime(datumTime);

  // /// Get the element's initial status content data
  // Magick::Image initialImage = GetInitialImageByDatumTime(&elementDatumTimeContent);

  /// Handle the element's initial status content by action status data
  if (!elementDatumTimeAction && -1 == elementStatusDataIndex) {  // Initial state
    /// datumTime = 0, Initial stata
    /// Get the element's initial status content data
    GetInitialImageByDatumTime(&elementDatumTimeContent, elementHandledInfo);
    if (ZIP_GIF == elementType_) {  // IMG
      VLOG(4) << "***** Inital status for elementType: " << elementType_
        << ", url = " << url_ << ", datumTime: " << datumTime;
    }
  } else if (!elementDatumTimeAction && -2 == elementStatusDataIndex) {
    /// datumTime != 0, empty actions (use the initial status data)
    /// Get the element's initial status content data
    GetInitialImageByDatumTime(&elementDatumTimeContent, elementHandledInfo);
    if (ZIP_GIF == elementType_) {  // IMG
      VLOG(4) << "***** EmptyActions, for elementType: " << elementType_
        << ", datumTime: " << datumTime;
    }
  } else if (!elementDatumTimeAction && -3 == elementStatusDataIndex) {
    /// datumTime != 0, not empty actions (use the initial status data)
    /// Get the element's initial status content data
    GetInitialImageByDatumTime(&elementDatumTimeContent, elementHandledInfo);
    if (ZIP_GIF == elementType_) {  // IMG
      VLOG(4) << "***** NotElementActions, for elementType: " << elementType_
        << ", datumTime: " << datumTime
        << ", xOff_ = " << elementHandledInfo.xOff_
        << ", yOff_ = " << elementHandledInfo.yOff_
        << ", width = " << elementHandledInfo.handledImg_.columns()
        << ", height = " << elementHandledInfo.handledImg_.rows();
    }
  } else {  // Intermediate state(Not initial state)
    if (ZIP_GIF == elementType_) {  // IMG
      VLOG(4) << "===== Intermediate state(Not initial state)"
        << ", actionType: " << elementDatumTimeAction->actionType_
        << ", datumTime: " << datumTime;
    }
    ActionType actionType = elementDatumTimeAction->actionType_;
    if (ROTATE == actionType) {
      MsgActionRotate* msgAction = (MsgActionRotate*)elementDatumTimeAction;
      double degree = msgAction->degrees_[elementStatusDataIndex];
      if (TEXT == elementType_) {  // IMG
        VLOG(4) << "Rotate degree: " << degree << ", datumTime: " << datumTime;
      }
      RotateByDatumTimeContent(&elementDatumTimeContent, degree,
            elementHandledInfo);
    } else if (SCALE == actionType) {
      MsgActionScale* msgAction = (MsgActionScale*)elementDatumTimeAction;
      double scale = msgAction->scales_[elementStatusDataIndex];
      if (TEXT == elementType_) {  // IMG
        VLOG(4) << "Scale factor: " << scale << ", datumTime: " << datumTime;
      }
      ScaleByDatumTimeContent(&elementDatumTimeContent, scale,
            elementHandledInfo);
    } else if (TRANSLATE == actionType) {
      MsgActionTranslate* msgAction = (MsgActionTranslate*)elementDatumTimeAction;
      int xOff = msgAction->translates_[elementStatusDataIndex].dx_;
      int yOff = msgAction->translates_[elementStatusDataIndex].dy_;
      if (TEXT == elementType_) {  // IMG
        VLOG(4) << "Translate: xOff = " << xOff << ", yOff = " << yOff << ", datumTime: " << datumTime;
      }
      TranslateByDatumTimeContent(&elementDatumTimeContent, xOff, yOff,
            elementHandledInfo);
    } else if (TEXT_ANIMATE == actionType) {
      GetInitialImageByDatumTime(&elementDatumTimeContent, elementHandledInfo);
      if (TEXT == elementType_) {
        VLOG(4) << "***** Intermediate status(TEXT_ANIMATE) for elementType: " << elementType_
          << ", url = " << url_ << ", datumTime: " << datumTime;
      }
    }
  }
  elementHandledInfo.isVisible_ = isElementVisible;
  return true;
}

bool MessageElement::HandleElementByActions(const int& datumTime,
      ElementHandledInfo<unsigned char>& elementHandledInfo) {
  if (VIDEO == elementType_) {
    //VLOG(4) << "VIDEO, datumTime: " << datumTime;
  }
  switch (elementType_) {
    case AUDIO_MEDIAPLAYER: {
      break;
    }
    case GIF: {
      HandleDatumTimeElement(datumTime, elementHandledInfo);
      break;
    }
    case ZIP_GIF: {
      HandleDatumTimeElement(datumTime, elementHandledInfo);
      break;
    }
    case IMG: {
      HandleDatumTimeElement(datumTime, elementHandledInfo);
      break;
    }
    case VIDEO: {
      /*
       * Only have PLAY action for VIDEO, handle here temporary
       */
      /// Get the element's datum-time content
      Magick::Image elementDatumTimeContent
        = elementContent_->GetContentByDatumTime(datumTime);
      /// Transform it by the Initial status
      GetInitialImageByDatumTime(&elementDatumTimeContent, elementHandledInfo);
      break;
    }
    case BACKGROUND_IMG: {
      HandleDatumTimeElement(datumTime, elementHandledInfo);
      break;
    }
    case TEXT: {
      HandleDatumTimeElement(datumTime, elementHandledInfo);
      break;
    }
    case UNKNOWN: {
      LOG(ERROR)
        << "Unknown ElementType when MessageElement::HandleElementByActions";
      return false;
    }
    default: {
      LOG(ERROR)
        << "No set ElementType when MessageElement::HandleElementByActions";
      return false;
    }
  }
  return true;
}
  
MsgAction* MessageElement::GetActionByDatumTime(const int& datumTime,
      int& statusDataIndex, bool& isVisible) {
  int retDataIndex = 0;
  bool retVisible = true;
  MsgAction* retMsgAction = NULL;
#if 0
  if (actions_.empty()) {  // No actions, always use the initial data
    retDataIndex = -1;     // Wrong index
    retMsgAction = NULL;
    retVisible = true;     // always isVisible = true;
  } else if (!actions_.empty() && 0 == datumTime) {
    retDataIndex = -1;     // Wrong index
    retMsgAction = NULL;
    for (std::vector<MsgAction*>::iterator it = actions_.begin();
          it != actions_.end(); ++it) {
      if (VISIBLE == (*it)->actionType_) {     // for VISIBLE type
        if ((*it)->startTime_ == datumTime) {  // datumTime = 0
          retVisible = (*it)->IsVisible();
          break;
        }
      }
    }
  } else {  // Have actions, datumTime != 0
    bool getStatusData = false;
    int finalActionIndex = 0;  // except VISIBLE action
    int index = 0;             // all actions' index
    for (std::vector<MsgAction*>::iterator it = actions_.begin();
          it != actions_.end(); ++it) {
      /// Get the datumTime's action visibility
      if (VISIBLE == (*it)->actionType_) {
        if (datumTime >= (*it)->startTime_) {
          retVisible = (*it)->IsVisible();
        }
      } else {
        /// Get the datumTime's action(except VISIBLE), its status data
        /// and the final Non-VISIBLE action index
        bool isFinalStatus = false;
        if (!getStatusData) {
          if (datumTime > (*it)->startTime_
                && datumTime <= (*it)->startTime_ + (*it)->duration_) {
            /// In an action's time segment
            getStatusData = true;  // set
            retMsgAction = *it;
            retDataIndex = retMsgAction->StatusDataIndexByDatum(
                  datumTime, isFinalStatus);
            VLOG(4) << "msgType: " << retMsgAction->actionType_
              << ", statusDataIndex: " << statusDataIndex
              << ", isFinalStatus: " << isFinalStatus;
          }
        }
        finalActionIndex = index;
      }
      index++;
    }
    /// The datumTime not in all actions time segment,
    /// use the final action's(except Non-VISIBLE) final status data
    if (!getStatusData) {
      bool isFinalStatus = false;
      retMsgAction = actions_[finalActionIndex];
      retDataIndex = retMsgAction->StatusDataIndexByDatum(
            datumTime, isFinalStatus);
      VLOG(4) << "msgType: " << retMsgAction->actionType_
        << ", statusDataIndex: " << statusDataIndex
        << ", isFinalStatus: " << isFinalStatus;
    }
  }
#endif
  /// -1: Initial state, -2: empty statusData, not initial state
  if (actions_.empty()) {  // actions empty
    if (0 == datumTime) {
      retDataIndex = -1;   // Initial state
      retMsgAction = NULL;
      retVisible = true;
    } else {  // datumTime isn't 0
      retDataIndex = -2;
      retMsgAction = NULL;
      retVisible = true;
    }
  } else {  // actions is not empty
    if (0 == datumTime) {
      retDataIndex = -1;   // Initial state
      retMsgAction = NULL;
      for (std::vector<MsgAction*>::iterator it = actions_.begin();
            it != actions_.end(); ++it) {
        if (VISIBLE == (*it)->actionType_) {  // for VISIBLE type
          if ((*it)->startTime_ == datumTime) {  // datumTime = 0
            retVisible = (*it)->IsVisible();
            break;
          } else if (datumTime < (*it)->startTime_) {
            /// set the 'Negated value'
            bool retFlag = ((MsgActionVisibility*)(*it))->IsVisible();
            if (retFlag) {  // true
              retVisible = false;
            } else {        // false
              retVisible = true;
            }
            break;
          }
        }
      }
    } else {  // datumTime != 0
      bool getStatusData = false;
      int finalActionIndex = 0;  // except VISIBLE action
      int index = 0;             // all actions' index
      bool isSetVisible = false;
      for (std::vector<MsgAction*>::iterator it = actions_.begin();
            it != actions_.end(); ++it) {
        /// Get the datumTime's action visibility
        if (VISIBLE == (*it)->actionType_ && !isSetVisible) {
          if (datumTime < (*it)->startTime_) {  // meet with the first startTime of VISIBLE
            bool retFlag = (*it)->IsVisible();
            if (retFlag) {  // true
              retVisible = false;
            } else {        // false
              retVisible = true;
            }
            isSetVisible = true;
          } else if (datumTime >= (*it)->startTime_) {
            retVisible = (*it)->IsVisible();
          }
        } else {  // Not visible action
          /// Get the datumTime's action(except VISIBLE), its status data
          /// and the final Non-VISIBLE action index
          bool isFinalStatus = false;
          if (!getStatusData) {
            if (datumTime > (*it)->startTime_
                  && datumTime <= (*it)->startTime_ + (*it)->duration_) {
              /// In this action's time segment
              getStatusData = true;  // have got the statusData
              retMsgAction = *it;
              retDataIndex = retMsgAction->StatusDataIndexByDatum(
                    datumTime, isFinalStatus);
              if (IMG == elementType_) {
              VLOG(5) << "actionType: " << retMsgAction->actionType_
                << ", statusDataIndex: " << statusDataIndex
                << ", isFinalStatus: " << isFinalStatus;
              }
            }
          }
          finalActionIndex = index;
        }
        index++;
      }
      /// The datumTime not in all actions time segment,
      /// use the final action's(except Non-VISIBLE) final status data
      if (!getStatusData) {
        bool isFinalStatus = false;
#if 0
        retMsgAction = actions_[finalActionIndex];
        retDataIndex = retMsgAction->StatusDataIndexByDatum(
              datumTime, isFinalStatus);
        VLOG(5) << "actionType: " << retMsgAction->actionType_
          << ", statusDataIndex: " << statusDataIndex
          << ", isFinalStatus: " << isFinalStatus;
#endif
        int actionIndex = -1;  // Must initialize use '-1'
        int index = 0;
        for (std::vector<MsgAction*>::iterator it = actions_.begin();
              it != actions_.end(); ++it) {
          if (VISIBLE != (*it)->actionType_) {  // Not a VISIBLE action
            if (datumTime > (*it)->startTime_ + (*it)->duration_) {
              actionIndex = index;
            }
          }
          index++;
        }
        if (actionIndex == -1) {
          retDataIndex = -3;   // datumTime != 0, not empty actions, but use the initial status
          retMsgAction = NULL;
          isFinalStatus = false;
        } else {
          retMsgAction = actions_[actionIndex];
          retDataIndex = retMsgAction->StatusDataIndexByDatum(
                datumTime, isFinalStatus);
        }
      }
    }
  }
#if 0
  /// TEST
  if (IMG == elementType_) {
    for (std::vector<MsgAction*>::iterator it = actions_.begin();
          it != actions_.end(); ++it) {
#if 0
      if (VISIBLE != (*it)->actionType_) {
        VLOG(4) << "actionType: " << (*it)->actionType_
          << ", startTime: " << (*it)->startTime_
          << ", duration: " << (*it)->duration_
          << ", totalTime: " << (*it)->startTime_ + (*it)->duration_
          << ", datumTime: " << datumTime;
      }
#endif
      if (VISIBLE == (*it)->actionType_) {
        VLOG(4) << "actionType: " << (*it)->actionType_
          << ", startTime: " << (*it)->startTime_
          << ", duration: " << (*it)->duration_
          << ", visible: " << ((MsgActionVisibility*)(*it))->visible_
          << ", datumTime: " << datumTime;
      }
    }
  }
#endif
  statusDataIndex = retDataIndex;
  isVisible = retVisible;
  return retMsgAction;
}
 
#if 0
Magick::Image MessageElement::GetInitialImageByDatumTime(
      Magick::Image* datumTimeImage) {
  int imgWidth = datumTimeImage->columns();
  int imgHeight = datumTimeImage->rows();
  int x = 0, y = 0, w = imgWidth, h = imgHeight;
  if (BIG_INT != x_ && x_ >= 0) {
    x = x_;
  }
  if (BIG_INT != y_ && y_ >= 0) {
    y = y_;
  }
  if (BIG_INT != w_ && w_ > 0) {
    w = w_;
  }
  if (BIG_INT != h_ && h_ > 0) {
    h = h_;
  }
  Magick::Image canvas;
  if (BIG_DOUBLE != scale_ && scale_ > 0) {
    canvas = ScaleByCenter(datumTimeImage, scale_,
          screenWidth_, screenHeight_, x, y);
  } else {
    canvas = Magick::Image(Magick::Geometry(screenWidth_, screenHeight_),
          Magick::Color("transparent"));
    canvas.backgroundColor("transparent");
    canvas.composite(*datumTimeImage,
          Magick::Geometry(w, h, x, y),
          Magick::OverCompositeOp);  // According to actual effect to change it
  }
  if (BIG_DOUBLE != degree_) {
    canvas.rotate(degree_);
  }
  return canvas;
}
#endif

void MessageElement::GetInitialImageByDatumTime(
      Magick::Image* datumTimeImage,
      ElementHandledInfo<unsigned char>& elementHandledInfo) {
  datumTimeImage->backgroundColor("transparent");
  int oW = datumTimeImage->columns();
  int oH = datumTimeImage->rows();
  int x = 0, y = 0, w = oW, h = oH;
  int xOff = 0, yOff = 0;
  if (BIG_INT != x_/* && x_ >= 0*/) {
    x = x_;
    xOff = x_;
  }
  if (BIG_INT != y_/* && y_ >= 0*/) {
    y = y_;
    yOff = y_;
  }
  if (BIG_INT != w_ && w_ > 0) {
    w = w_;
  }
  if (BIG_INT != h_ && h_ > 0) {
    h = h_;
  }

#if 0
  if (BACKGROUND_IMG  == elementType_) {
    VLOG(4) << "ORIGINAL| x = " << x << ", y = " << y << ", w = " << w << ", h = " << h;
  }
#endif
  
  /// Initial scale by center
  if (BIG_DOUBLE != scale_ && scale_ > 0) {
    int dx = (oW - oW * scale_) / 2;
    int dy = (oH - oH * scale_) / 2;
    x = x + dx;
    y = y + dy;
    datumTimeImage->scale(Magick::Geometry(oW * scale_, oH * scale_));
  }

#if 0
  if (BACKGROUND_IMG  == elementType_) {
    VLOG(4) << "SCLAE| x = " << x << ", y = " << y;
  }
#endif
  /// Initial rotate by center
  if (BIG_DOUBLE != degree_ && VIDEO != elementType_) {  // Don't rotate VIDEO element
    oW = datumTimeImage->columns();
    oH = datumTimeImage->rows();
    datumTimeImage->rotate(degree_);
    int cW = datumTimeImage->columns();
    int cH = datumTimeImage->rows();
    x = x - (cW - oW) / 2;
    y = y - (cH - oH) / 2;
  }

#if 0
  if (BACKGROUND_IMG  == elementType_) {
    VLOG(4) << "ROTATE| x = " << x << ", y = " << y;
  }
#endif
  elementHandledInfo.handledImg_ = *datumTimeImage;
  if (BACKGROUND_IMG  == elementType_) {
    elementHandledInfo.xOff_ = xOff;  // x
    elementHandledInfo.yOff_ = yOff;  // y;
  } else {
    elementHandledInfo.xOff_ = x;  // x;
    elementHandledInfo.yOff_ = y;  // y;
  }
}

void MessageElement::RotateByDatumTimeContent( 
    Magick::Image* datumTimeImage,
    const double& degree,
    ElementHandledInfo<unsigned char>& elementHandledInfo) {
  datumTimeImage->backgroundColor("transparent");
  int x = 0, y = 0;
  if (BIG_INT != x_ && x_ >= 0) {
    x = x_;
  }
  if (BIG_INT != y_ && y_ >= 0) {
    y = y_;
  }
  int oW = datumTimeImage->columns();
  int oH = datumTimeImage->rows();
  datumTimeImage->rotate(degree);
  int cW = datumTimeImage->columns();
  int cH = datumTimeImage->rows();
  elementHandledInfo.handledImg_ = *datumTimeImage;
  elementHandledInfo.xOff_ = x - (cW - oW) / 2;
  elementHandledInfo.yOff_ = y - (cH - oH) / 2;
}

void MessageElement::ScaleByDatumTimeContent( 
    Magick::Image* datumTimeImage,
    const double& scale,
    ElementHandledInfo<unsigned char>& elementHandledInfo) {
  datumTimeImage->backgroundColor("transparent");
  int x = 0, y = 0;
  if (BIG_INT != x_ && x_ >= 0) {
    x = x_;
  }
  if (BIG_INT != y_ && y_ >= 0) {
    y = y_;
  }
  int oW = datumTimeImage->columns();
  int oH = datumTimeImage->rows();
  datumTimeImage->scale(Magick::Geometry(oW * scale, oH * scale));
  int dx = (oW - oW * scale) / 2;
  int dy = (oH - oH * scale) / 2;
  elementHandledInfo.handledImg_ = *datumTimeImage;
  elementHandledInfo.xOff_ = x + dx;
  elementHandledInfo.yOff_ = y + dy;
}
  
void MessageElement::TranslateByDatumTimeContent( 
    Magick::Image* datumTimeImage,
    const int& xOff, const int& yOff,
    ElementHandledInfo<unsigned char>& elementHandledInfo) {
  datumTimeImage->backgroundColor("transparent");
  elementHandledInfo.handledImg_ = *datumTimeImage;
  elementHandledInfo.xOff_ = xOff;
  elementHandledInfo.yOff_ = yOff;
  x_ = xOff;
  y_ = yOff;
}
}  // namespace multimedia
}  // namespace juyou
