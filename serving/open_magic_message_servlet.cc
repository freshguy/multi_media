// Copyright (c) 2013 Juyou Inc. All rights reserved.
// Author: wangjuntao@juyou.com (Juntao Wang)

#include "juyou/multi_media/serving/open_magic_message_servlet.h"

namespace juyou {
namespace multimedia {

OpenMagicMessageServletHandler::OpenMagicMessageServletHandler() {
  open_magic_message_handler_ = NULL;
  open_magic_message_handler_ = new OpenMagicMessageHandler;
}

OpenMagicMessageServletHandler::~OpenMagicMessageServletHandler() {
  if (open_magic_message_handler_) {
    delete open_magic_message_handler_;
  }
  open_magic_message_handler_ = NULL;
}

void OpenMagicMessageServletHandler::ShareMagicMessage(
      MessageShareResponse& _return,
      const MessageShareRequest& messageShareRequest) {
  VLOG(4) << "[RPC] ShareMagicMessage...";
  if (!open_magic_message_handler_->ShareMagicMessageInternal(
          _return, messageShareRequest)) {
    LOG(ERROR) << "[RPC] SharedMagicMessage error";
  }
}
}  // namespace multimedia
}  // namespace juyou
