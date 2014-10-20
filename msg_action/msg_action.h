// Copyright (c) 2013 Juyou Inc. All rights reserved.
// Author: wangjuntao@juyou.com (Juntao Wang)

#ifndef JUYOU_MULTI_MEDIA_MSG_ACTION_MSG_ACTION_H_
#define JUYOU_MULTI_MEDIA_MSG_ACTION_MSG_ACTION_H_

#include <string>
#include <vector>
#include "base/logging.h"
#include "base/flags.h"
#include "base/time.h"
#include "base/string_util.h"
#include "third_party/jsoncpp/include/json.h"

namespace juyou {
namespace multimedia {

enum ActionType {
  ADJUST_VOLUMN = 1,
  PLAY = 2,
  ROTATE = 3,
  SCALE = 4,
  // TEXT = 5,
  TRANSLATE = 6,
  VIBRATE = 7,
  VISIBLE = 8,
  TEXT_ANIMATE = 9,
};

class MsgAction {
 public:
  MsgAction() {
    startTime_ = 0;
    duration_ = 0;
    isRunning_ = false;
  }
  virtual ~MsgAction() {
  }

  virtual void SetActionType() = 0;
  virtual bool ParseActionStatusData(
        const Json::Value& data_json) = 0;

  virtual int StatusDataIndexByDatum(const int& datumTime,
        bool& isFinalStatus) = 0;
  
  /// Only the VISIBLE type MsgAction override the IsVisible()
  virtual bool IsVisible() {
    return true;
  }

 public:
  ActionType actionType_;
  int startTime_;
  int duration_;
  bool isRunning_;
};
}  // namespace multimedia
}  // namespace juyou
#endif  // JUYOU_MULTI_MEDIA_MSG_ACTION_MSG_ACTION_H_
