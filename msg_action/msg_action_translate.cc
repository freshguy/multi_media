// Copyright (c) 2013 Juyou Inc. All rights reserved.
// Author: wangjuntao@juyou.com (Juntao Wang)

#include "juyou/multi_media/msg_action/msg_action_translate.h"

namespace juyou {
namespace multimedia {

MsgActionTranslate::MsgActionTranslate() {
}

MsgActionTranslate::~MsgActionTranslate() {
}

void MsgActionTranslate::SetActionType() {
  actionType_ = TRANSLATE;
}

bool MsgActionTranslate::ParseActionStatusData(
      const Json::Value& data_json) {
  VLOG(4) << "ParseActionStatusData for ActionType::TRANSLATE";
  if (data_json.empty()) {
    LOG(ERROR)
      << "Action data_json empty for ActionType::TRANSLATE";
    return false;
  }
  if (!data_json.isMember("tracks")) {
    LOG(ERROR)
      << "No feilds tracks in data_json for ActionType::TRANSLATE";
    return false;
  }
  if (data_json.isMember("startTime")) {
    startTime_ = data_json["startTime"].asInt();
  }
  if (data_json.isMember("duration")) {
    duration_ = data_json["duration"].asInt();
  }
  for (int i = 0; i < data_json["tracks"].size(); i++) {
    TranslateDelta translate;
    translate.dx_ = data_json["tracks"][i]["x"].asDouble();
    translate.dy_ = data_json["tracks"][i]["y"].asDouble();
    translates_.push_back(translate);
  }
  VLOG(4) << "Have " << translates_.size()
    << " translate status data";
  return true;
}
}  // namespace multimedia
}  // namespace juyou
