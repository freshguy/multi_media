// Copyright (c) 2013 Juyou Inc. All rights reserved.
// Author: wangjuntao@juyou.com (Juntao Wang)

#ifndef JUYOU_MULTI_MEDIA_MSG_ACTION_MSG_ACTION_VISIBILITY_H_
#define JUYOU_MULTI_MEDIA_MSG_ACTION_MSG_ACTION_VISIBILITY_H_

#include "juyou/multi_media/msg_action/msg_action.h"

namespace juyou {
namespace multimedia {

class MsgActionVisibility : public MsgAction {
 public:
  MsgActionVisibility ();
  virtual ~MsgActionVisibility ();
  
  virtual void SetActionType();
  virtual bool ParseActionStatusData(
        const Json::Value& data_json);

  virtual int StatusDataIndexByDatum(const int& datumTime,
        bool& isFinalStatus) {
    isFinalStatus = false;  // in this range represent not be finalStatus
    return -1;  // No have status index
  }

  virtual bool IsVisible();
 public:
  bool visible_;
 private:
};
}  // namespace multimedia
}  // namespace juyou
#endif  // JUYOU_MULTI_MEDIA_MSG_ACTION_MSG_ACTION_VISIBILITY_H_
