// Copyright (c) 2013 Juyou Inc. All rights reserved.
// Author: wangjuntao@juyou.com (Juntao Wang)

#ifndef JUYOU_MULTI_MEDIA_SERVING_OPEN_MAGIC_MESSAGE_SERVLET_H_
#define JUYOU_MULTI_MEDIA_SERVING_OPEN_MAGIC_MESSAGE_SERVLET_H_

#include <string>
#include <vector>
#include "base/logging.h"
#include "base/flags.h"
#include "juyou/multi_media/proto/gen-cpp/OpenMagicMessageServlet.h"
#include "juyou/multi_media/public/open_magic_message_handler.h"

using boost::shared_ptr;

namespace juyou {
namespace multimedia {

class OpenMagicMessageServletHandler : virtual public OpenMagicMessageServletIf {
 public:
  OpenMagicMessageServletHandler();
  virtual ~OpenMagicMessageServletHandler();

  void ShareMagicMessage(MessageShareResponse& _return,
        const MessageShareRequest& messageShareRequest);
 private:
  OpenMagicMessageHandler* open_magic_message_handler_;
};
}  // namespace multimedia
}  // namespace juyou
#endif  // JUYOU_MULTI_MEDIA_SERVING_OPEN_MAGIC_MESSAGE_SERVLET_H_
