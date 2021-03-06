// Copyright (c) 2013 Juyou Inc. All rights reserved.
// Author: wangjuntao@juyou.com (Juntao Wang)

#ifndef JUYOU_MULTI_MEDIA_MSG_ACTION_MSG_ACTION_SCALE_H_
#define JUYOU_MULTI_MEDIA_MSG_ACTION_MSG_ACTION_SCALE_H_

#include "juyou/multi_media/msg_action/msg_action.h"

namespace juyou {
namespace multimedia {

class MsgActionScale : public MsgAction {
 public:
  MsgActionScale();
  virtual ~MsgActionScale();
  
  virtual void SetActionType();
  virtual bool ParseActionStatusData(
        const Json::Value& data_json);

  virtual int StatusDataIndexByDatum(const int& datumTime,
        bool& isFinalStatus) {
    int index = 0;
    if (datumTime > startTime_
          && datumTime <= startTime_ + duration_) {
      int segTime = duration_ / scales_.size();
      int currentTime = startTime_;
      for (int i = 0; i < scales_.size(); i++) {
        currentTime += segTime;
        if (datumTime <= currentTime) {
          if (i == 0) {
            index = 0;
          } else {
            if ((currentTime - datumTime) <= (datumTime - (currentTime - segTime))) {
              index = i;
            } else {
              index = i - 1;
            }
          }
          break;
        }
      }
      isFinalStatus = false;
    } else {
      index = scales_.size() - 1;
      isFinalStatus = true;
    }
    return index;
  }

 public:
  std::vector<double> scales_;
};
}  // namespace multimedia
}  // namespace juyou
#endif  // JUYOU_MULTI_MEDIA_MSG_ACTION_MSG_ACTION_SCALE_H_
