// Copyright (c) 2013 Juyou Inc. All rights reserved.
// Author: wangjuntao@juyou.com (Juntao Wang)

#ifndef JUYOU_MULTI_MEDIA_SERVING_OPEN_MAGIC_MESSAGE_SERVER_H_
#define JUYOU_MULTI_MEDIA_SERVING_OPEN_MAGIC_MESSAGE_SERVER_H_

#include "news_search/base/general_server/general_server.h"
#include "juyou/multi_media/serving/open_magic_message_servlet.h"

#define OPEN_MAGIC_MESSAGE_SERVER_MONITOR_VALIDATE_KEY ("Open Magic Message Server")

namespace juyou {
namespace multimedia {

class OpenMagicMessageServer {
 public:
  OpenMagicMessageServer();
  virtual ~OpenMagicMessageServer();
  
  void Serve();
 private:
  void ValidateMonitorOpenMagicMessageServer(
        babysitter::MonitorResult* result);

 private:
  news::GeneralServer* general_server_;
};
}  // namespace multimedia
}  // namespace juyou
#endif  // JUYOU_MULTI_MEDIA_SERVING_OPEN_MAGIC_MESSAGE_SERVER_H_
