// Copyright (c) 2013 Juyou Inc. All rights reserved.
// Author: wangjuntao@juyou.com (Juntao Wang)

#include "juyou/multi_media/msg_action/msg_action_visibility.h"

namespace juyou {
namespace multimedia {

MsgActionVisibility::MsgActionVisibility() {
}

MsgActionVisibility::~MsgActionVisibility() {
}

void MsgActionVisibility::SetActionType() {
  // set_action_type(VISIBLE);
  actionType_ = VISIBLE;
}

bool MsgActionVisibility::IsVisible() {
  return visible_;
}

bool MsgActionVisibility::ParseActionStatusData(
      const Json::Value& data_json) {
  VLOG(4) << "ParseActionStatusData for ActionType::VISIBLE";
  if (data_json.empty()) {
    LOG(ERROR)
      << "Action data_json empty for ActionType::VISIBLE";
    return false;
  }
  if (data_json.isMember("startTime")) {
    startTime_ = data_json["startTime"].asInt();
  }
  if (data_json.isMember("duration")) {
    duration_ = data_json["duration"].asInt();
  }
  if (data_json.isMember("visible")) {
    visible_ = data_json["visible"].asBool();
  }
  return true;
}
}  // namespace multimedia
}  // namespace juyou
