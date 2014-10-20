// Copyright (c) 2013 Juyou Inc. All rights reserved.
// Author: wangjuntao@juyou.com (Juntao Wang)

#include "juyou/multi_media/msg_action/msg_action_play.h"

namespace juyou {
namespace multimedia {

MsgActionPlay::MsgActionPlay() {
}

MsgActionPlay::~MsgActionPlay() {
}

void MsgActionPlay::SetActionType() {
  actionType_ = PLAY;
}

bool MsgActionPlay::ParseActionStatusData(
      const Json::Value& data_json) {
  VLOG(4) << "ParseActionStatusData for ActionType::PLAY";
  if (data_json.empty()) {
    LOG(ERROR)
      << "Action data_json empty for ActionType::PLAY";
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
