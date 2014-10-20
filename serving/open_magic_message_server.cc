// Copyright (c) 2013 Juyou Inc. All rights reserved.
// Author: wangjuntao@juyou.com (Juntao Wang)

#include "juyou/multi_media/serving/open_magic_message_server.h"

namespace juyou {
namespace multimedia {

OpenMagicMessageServer::OpenMagicMessageServer() {
  general_server_ = NULL;
}

OpenMagicMessageServer::~OpenMagicMessageServer() {
  if (general_server_) {
    delete general_server_;
  }
  general_server_ = NULL;
}

void OpenMagicMessageServer::Serve() {
  shared_ptr<OpenMagicMessageServletHandler> handler(
        new OpenMagicMessageServletHandler());
  shared_ptr<TProcessor> processor(
        new OpenMagicMessageServletProcessor(handler));

  /// In GeneralServer(): create monitor_server
  general_server_ = new news::GeneralServer(processor);
  
  LOG(INFO) << "Add monitor...";
  /// Add Monitor
  general_server_->AddMonitor(
        OPEN_MAGIC_MESSAGE_SERVER_MONITOR_VALIDATE_KEY,
        base::NewPermanentCallback(
          this,
          &OpenMagicMessageServer::ValidateMonitorOpenMagicMessageServer));
  /// Start the server
  LOG(INFO) << "Starting GeneralServer Instance for OpenMagicMessageServer...";
  
  bool isStart = true;
  try {
    general_server_->Start();
  } catch(const ::apache::thrift::TException& tx) {
    LOG(ERROR) << "Start GeneralServer failed(0): " << tx.what();
    isStart = false;
  } catch(...) {
    LOG(ERROR) << "Failed to start GeneralServer, unexplained exception";
    isStart = false;
  }
  CHECK(isStart) << "Failed to Start OpenMagicMessageServer";
  LOG(INFO) << "Start OpenMagicMessageServer successfully";
}

void OpenMagicMessageServer::ValidateMonitorOpenMagicMessageServer(
      babysitter::MonitorResult* result) {
  std::string monitor_info;
#if 0
  if (false == handler_->IsHealthy) {
    monitor_info = "Bad";
  } else {
    monitor_info = "Good";
  }
#endif
  monitor_info = "Everything is OK";
  result->AddKv("Open Magic Message Server Monitor", monitor_info);
}
}  // namespace multimedia
}  // namespace juyou
