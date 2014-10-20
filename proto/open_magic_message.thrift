// Copyright 2013 Juyou Inc. All Rights Reserved.
// Author: wangjuntao@juyou.com (Juntao Wang)

include "juyou/circles/proto/common_info.thrift"

namespace cpp juyou.multimedia
namespace php Juyou_multimedia
namespace java juyou.multimedia

enum ReturnCode {
  SUCCESSFUL = 1,
  BAD_PARAMETER = 2,
  ERRORS = 3,
  XML_CONTENT_ERRORS = 4,
  STORAGE_ERRORS = 5,
}

struct MagicMessage {
  1: string messageXmlContent,
}

struct OpenMagicMessage {
  1: string messageContentUrl,
}

struct MessageShareRequest {
  1: common_info.UserInfo sharerInfo,
  2: MagicMessage magicMessage,
}

struct MessageShareResponse {
  1: ReturnCode returnCode,
  2: OpenMagicMessage openMagicMessage,
}

service OpenMagicMessageServlet {
  MessageShareResponse ShareMagicMessage(
        1: MessageShareRequest messageShareRequest);
}
