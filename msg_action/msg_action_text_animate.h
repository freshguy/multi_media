// Copyright (c) 2013 Juyou Inc. All rights reserved.
// Author: wangjuntao@juyou.com (Juntao Wang)

#ifndef JUYOU_MULTI_MEDIA_MSG_ACTION_MSG_ACTION_TEXT_ANIMATE_H_
#define JUYOU_MULTI_MEDIA_MSG_ACTION_MSG_ACTION_TEXT_ANIMATE_H_

#include "juyou/multi_media/msg_action/msg_action.h"

namespace juyou {
namespace multimedia {

class MsgActionTextAnimate : public MsgAction {
 public:
  MsgActionTextAnimate();
  virtual ~MsgActionTextAnimate();

  virtual void SetActionType();
  virtual bool ParseActionStatusData(
        const Json::Value& data_json);
  
  virtual int StatusDataIndexByDatum(const int& datumTime,
        bool& isFinalStatus) {
    if (datumTime > startTime_
          && datumTime <= startTime_ + duration_) {
      /// datumTime in this time segment, not the finalStatus
      isFinalStatus = false;
    } else {
      isFinalStatus = true;
    }
    return -1;  // No have status index
  }
 public:
};
}  // namespace multimedia
}  // namespace juyou
#endif  // JUYOU_MULTI_MEDIA_MSG_ACTION_MSG_ACTION_TEXT_ANIMATE_H_
