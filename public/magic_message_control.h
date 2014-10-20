// Copyright (c) 2013 Juyou Inc. All rights reserved.
// Author: wangjuntao@juyou.com (Juntao Wang)

#ifndef JUYOU_MULTI_MEDIA_PUBLIC_MAGIC_MESSAGE_CONTROL_H_
#define JUYOU_MULTI_MEDIA_PUBLIC_MAGIC_MESSAGE_CONTROL_H_

#include <string>
#include <vector>
#include <algorithm>
#include "base/logging.h"
#include "base/flags.h"
#include "base/thread.h"
#include "base/mutex.h"
#include "base/thrift.h"
#include "base/basictypes.h"
#include "third_party/jsoncpp/include/json.h"
#include "news_search/base/timer/timer.h"
#include "juyou/circles/proto/gen-cpp/RemoteFileHandleServlet.h"
#include "juyou/multi_media/public/message_element.h"
#include "juyou/multi_media/public/handle_element_thread.h"

using boost::shared_ptr;  // NOLINT

DECLARE_string(multimedia_file_dir);

namespace juyou {
namespace multimedia {

enum AudioFormat {
  AMR = 1,
  OGG = 2,
  MP3 = 3,
  WAV = 4,
};

struct AudioHandledInfo {
  AudioHandledInfo() {
    isHandled_ = false;
  }
  std::string origAudioFileName;
  AudioFormat origAudioFormat;
  std::string origAudioFilePath;
  std::string handledAudioFilePath;
  AudioFormat handledAudioFormat;  // Uniform format: MP3
  bool isHandled_;
};

class MagicMessageControlRule {
 public:
  MagicMessageControlRule() {
    screenWidth_ = 0;
    screenHeight_ = 0;
    duration_ = 0;
  }
  ~MagicMessageControlRule() {
    for (std::vector<MessageElement*>::iterator it = messageElements_.begin();
          it != messageElements_.end(); ++it) {
      if (*it) {
        delete *it;
      }
      *it = NULL;
    }
    messageElements_.clear();
  }
 public:
  int screenWidth_;
  int screenHeight_;
  std::string baseUrl_;
  int duration_;
  std::vector<MessageElement*> messageElements_;
};

class MagicMessageControl {
 public:
  MagicMessageControl();
  ~MagicMessageControl();

  /// step1:
  bool ExtractMessageControlRule(
        const std::string& json_str);
  /// step2:
  bool SetMessageElementContents(
        const std::map<std::string, RemoteFileInfo>& elementNameContentMap);
  /// step3:
  bool MagicMessage2Video(std::string& magicMessageVideoContent);

  /// getter and setter
  MagicMessageControlRule& get_magic_message_control_rule() {
    return magic_message_control_rule_;
  }
 
 private:
  bool ExtractElement(const Json::Value& elements);
  bool ExtractAction(const Json::Value& actions_json,
        std::vector<MsgAction*>& actions);

  bool AudioSynthesize(const std::string& audioFileDir,
        std::map<MessageElement*, AudioHandledInfo>& inputAudioFragmentInfo,
        AudioHandledInfo& handledAudioInfo);
 
 private:
  MagicMessageControlRule magic_message_control_rule_;
};

std::string GetFileExtentionName(const std::string& fileName);

bool CompareMsgActionByStartTimeASC(const MsgAction* msgPtr1,
      const MsgAction* msgPtr2);

template<class T>
int MaxFinalTimeStamp(
      const std::map<int, ElementHandledInfo<T> >& idElementHandledMap);

void StringToLowerCase(std::string& str);
}  // namespace multimedia
}  // juyou
#endif  // JUYOU_MULTI_MEDIA_PUBLIC_MAGIC_MESSAGE_CONTROL_H_
