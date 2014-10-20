// Copyright (c) 2013 Juyou Inc. All rights reserved.
// Author: wangjuntao@juyou.com (Juntao Wang)

#include "juyou/multi_media/msg_action/msg_action_rotate.h"

namespace juyou {
namespace multimedia {

MsgActionRotate::MsgActionRotate() {
}

MsgActionRotate::~MsgActionRotate() {
}

void MsgActionRotate::SetActionType() {
  actionType_ = ROTATE;
}

bool MsgActionRotate::ParseActionStatusData(
      const Json::Value& data_json) {
  VLOG(4) << "ParseActionStatusData for ActionType::ROTATE";
  if (data_json.empty()) {
    LOG(ERROR)
      << "Action data_json empty for ActionType::ROTATE";
    return false;
  }
  if (!data_json.isMember("tracks")) {
    LOG(ERROR)
      << "No feilds tracks in data_json for ActionType::ROTATE";
    return false;
  }
  if (data_json.isMember("startTime")) {
    startTime_ = data_json["startTime"].asInt();
  }
  if (data_json.isMember("duration")) {
    duration_ = data_json["duration"].asInt();
  }
  for (int i = 0; i < data_json["tracks"].size(); i++) {
    degrees_.push_back(data_json["tracks"][i].asDouble());
  }
  VLOG(4) << "Have " << degrees_.size()
    << " rotate status data";
  return true;
}
}  // namespace multimedia
}  // namespace juyou
