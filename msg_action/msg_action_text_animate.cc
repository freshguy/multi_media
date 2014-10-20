// Copyright (c) 2013 Juyou Inc. All rights reserved.
// Author: wangjuntao@juyou.com (Juntao Wang)

#include "juyou/multi_media/msg_action/msg_action_text_animate.h"

namespace juyou {
namespace multimedia {

MsgActionTextAnimate::MsgActionTextAnimate() {
}  

MsgActionTextAnimate::~MsgActionTextAnimate() {
}

void MsgActionTextAnimate::SetActionType() {
  actionType_ = TEXT_ANIMATE;
}

bool MsgActionTextAnimate::ParseActionStatusData(
      const Json::Value& data_json) {
  VLOG(4) << "ParseActionStatusData for ActionType::TEXT_ANIMATE";
  if (data_json.empty()) {
    LOG(ERROR)
      << "Action data_json empty for ActionType::TEXT_ANIMATE";
    return false;
  }
  if (data_json.isMember("startTime")) {
    startTime_ = data_json["startTime"].asInt();
  }
  if (data_json.isMember("duration")) {
    duration_ = data_json["duration"].asInt();
  }
  return true;
}
}  // namespace multimedia
}  // namespace juyou
