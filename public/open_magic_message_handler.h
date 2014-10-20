// Copyright (c) 2013 Juyou Inc. All rights reserved.
// Author: wangjuntao@juyou.com (Juntao Wang)

#ifndef JUYOU_MULTI_MEDIA_PUBLIC_OPEN_MAGIC_MESSAGE_HANDLER_H_
#define JUYOU_MULTI_MEDIA_PUBLIC_OPEN_MAGIC_MESSAGE_HANDLER_H_

#include <string>
#include <vector>
#include <deque>
#include "base/logging.h"
#include "base/flags.h"
#include "base/basictypes.h"
#include "base/thrift.h"
#include "base/thread.h"
#include "base/mutex.h"
#include "third_party/tinyxml/tinyxml.h"
#include "juyou/multi_media/proto/gen-cpp/OpenMagicMessageServlet.h"
#include "juyou/storage/proto/gen-cpp/FileTransferServiceServlet.h"
#include "juyou/multi_media/public/magic_message_control.h"

using boost::shared_ptr;  // NOLINT
using ::juyou::storage::FileTransferServiceServletClient;  // NOLINT
using ::juyou::circles::RemoteFileHandleServletClient;  // NOLINT

#define DEFAULT_TFSKEY_NODE_CNT 3  // Don't modify this macro unless you know the reason
#define TEMPLATE_LOCAL_NAME "template_1.txt"

DECLARE_string(file_transfer_server_host);
DECLARE_int32(file_transfer_server_port);

/// for remote_file_server
DECLARE_string(remote_file_server_host);
DECLARE_int32(remote_file_server_port);

namespace juyou {
namespace multimedia {

struct MagicMessageHandleInfo {
  MagicMessageControl* magic_message_control_;
  // std::string message_content_url_;
  ::juyou::circles::LocalFileInfo localFileInfo_;
};

class MagicMessageHandleThread;
class OpenMagicMessageHandler {
 public:
  OpenMagicMessageHandler();
  ~OpenMagicMessageHandler();

  bool ShareMagicMessageInternal(
        MessageShareResponse& _return,
        const MessageShareRequest& messageShareRequest);

 public:
  /// setter and getter
  base::ThriftClient<RemoteFileHandleServletClient>* get_remote_file_handle_servlet_client() {
    return remote_file_server_client_.get();
  }

  std::deque<MagicMessageHandleInfo>& get_magic_message_infos() {
    return magicMessageHandleInfos_;
  }

  base::Mutex& get_magic_message_handle_mutex() {
    return magicMessagesHandleMutex_;
  }

  base::CondVar& get_magic_message_handle_condvar() {
    return magicMessagesHandleCondVar_;
  }

 private:
  boost::shared_ptr<base::ThriftClient<
    FileTransferServiceServletClient> > file_transfer_servlet_client_;

  /// for RemoteFileHandleServletClient
  boost::shared_ptr<base::ThriftClient<
    RemoteFileHandleServletClient> > remote_file_server_client_;

  /// 
  std::deque<MagicMessageHandleInfo> magicMessageHandleInfos_;
  base::Mutex magicMessagesHandleMutex_;
  base::CondVar magicMessagesHandleCondVar_;
  MagicMessageHandleThread* magicMessageHandleThread_;
  
  DISALLOW_COPY_AND_ASSIGN(OpenMagicMessageHandler);
};

/// 
class MagicMessageHandleThread : public base::Thread {
 public:
   MagicMessageHandleThread(
         OpenMagicMessageHandler* openMagicMessageHandler);
   virtual ~MagicMessageHandleThread();
   virtual void Run();
 private:
   OpenMagicMessageHandler* openMagicMessageHandler_;
};
}  // namespace multimedia
}  // namespace juyou
#endif  // JUYOU_MULTI_MEDIA_PUBLIC_OPEN_MAGIC_MESSAGE_HANDLER_H
