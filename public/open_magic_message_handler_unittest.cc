// Copyright (c) 2013 Juyou Inc. All rights reserved.
// Author: wangjuntao@juyou.com (Juntao Wang)

#include <string>
#include <vector>
#include "base/logging.h"
#include "base/thrift.h"
#include "base/flags.h"
#include "file/file.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "juyou/multi_media/public/open_magic_message_handler.h"

//std::string messageXmlContent = "<story> <cachepath>T1pthTByCQ1RCvBVdK</cachepath><cachename></cachename><x>-1</x><y>-1</y><width>-1</width><height>-1</height><startscriptname>PROCESS</startscriptname><ftpkey><name>6716737ccf414abca88e51820a7ffb42_frame.png</name><value>T1pyhTBy_Q1RCvBVdK</value><localvalue></localvalue></ftpkey><ftpkey><name>template_1.txt</name><value>T1pthTByCQ1RCvBVdK</value><localvalue></localvalue></ftpkey><ftpkey><name>6716737ccf414abca88e51820a7ffb42_user_1.zip</name><value>T1FthTBsZj1RCvBVdK</value><localvalue></localvalue></ftpkey><ftpkey><name>6716737ccf414abca88e51820a7ffb42_user_2.zip</name><value>T1RahTBsxj1RCvBVdK</value><localvalue></localvalue></ftpkey></story>";

//std::string messageXmlContent = "<?xml version='1.0' encoding='UTF-8' standalone='yes' ?><story><cachepath>2013_12_09__23_07_34850d6693-66de-4164-bb70-5fd3312d5d82_template_1</cachepath><cachename></cachename><x>-1</x><y>-1</y><width>-1</width><height>-1</height><startscriptname>PROCESS</startscriptname><ftpkey><name>template_1.txt</name><value>T17aETBXDT1RCvBVdK</value><localvalue>&lt;![CDATA[]]&gt;</localvalue></ftpkey><ftpkey><name>38c5a2d0-3602-4953-9066-5d3cfaba43b5.Ogg</name><value>T1pRhTBsWj1RCvBVdK</value><localvalue>&lt;![CDATA[]]&gt;</localvalue></ftpkey><ftpkey><name>2013_12_09__23_04_24a4dd1c9d-36fd-4f8d-9d6a-1247c901acec.png</name><value>T17RETBXxT1RCvBVdK</value><localvalue>&lt;![CDATA[]]&gt;</localvalue></ftpkey><ftpkey><name>MP4_2013_12_09__22_58_3129045d8b-e0a6-4474-a6c3-3fe9de0e48cc.zip</name><value>T17yETBXdT1RCvBVdK</value><localvalue>&lt;![CDATA[]]&gt;</localvalue></ftpkey><ftpkey><name>38c5a2d0-3602-4953-9066-5d3cfaba43b5.gif</name><value>T1KyETBXWT1RCvBVdK</value><localvalue>&lt;![CDATA[]]&gt;</localvalue></ftpkey></story>";

//std::string messageXmlContent = "<?xml version=1.0 encoding='UTF-8'?><story><cachepath>2013_12_19__13_47_43ea7116ac-a982-46d2-ab05-14267e730992_template_1</cachepath><cachename/><x>-1</x><y>-1</y><width>-1</width><height>-1</height><startscriptname>PROCESS</startscriptname><ftpkey><name>template_1.txt</name><value>T1_yETBXDT1RCvBVdK</value><localvalue/></ftpkey><ftpkey><name>2013_12_19__13_46_18de3c7057-fc32-44f8-bc2b-9d0a0968f5a5.jpg</name><value>T1_tETBXxT1RCvBVdK</value><localvalue/></ftpkey><ftpkey><name>20131219134623.mp4</name><value>T1gaETBXhT1RCvBVdK</value><localvalue/></ftpkey></story>";

// std::string messageXmlContent = "<?xml version='1.0' encoding='utf8'?><story> <cachepath>T1XRETBydT1RCvBVdK</cachepath><cachename></cachename><x>-1</x><y>-1</y><width>-1</width><height>-1</height><startscriptname>PROCESS</startscriptname><ftpkey><name>804fd3f9-e1d0-48b7-b1e3-042e9b6bd211.ogg</name><value>T12ahTBQKj1RCvBVdK</value><localvalue></localvalue></ftpkey><ftpkey><name>2013_09_18__22_52_07136be89c-8fe6-4d8b-97d2-19f981b323d4.zip</name><value>T1dyhTBQVj1RCvBVdK</value><localvalue></localvalue></ftpkey><ftpkey><name>template_1.txt</name><value>T1TyETBybT1RCvBVdK</value><localvalue></localvalue></ftpkey></story>";

std::string messageXmlContent = "<?xml version='1.0' encoding='utf8'?><story> <cachepath>T1XRETBydT1RCvBVdK</cachepath><cachename></cachename><x>-1</x><y>-1</y><width>-1</width><height>-1</height><startscriptname>PROCESS</startscriptname><ftpkey><name>804fd3f9-e1d0-48b7-b1e3-042e9b6bd211.ogg</name><value>T12ahTBQKj1RCvBVdK</value><localvalue></localvalue></ftpkey><ftpkey><name>2013_09_18__22_52_07136be89c-8fe6-4d8b-97d2-19f981b323d4.zip</name><value>T1dyhTBQVj1RCvBVdK</value><localvalue></localvalue></ftpkey><ftpkey><name>template_1.txt</name><value>T1TyETByVT1RCvBVdK</value><localvalue></localvalue></ftpkey></story>";

using boost::shared_ptr;

namespace juyou {
namespace multimedia {
TEST(OpenMagicMessageHandler, ShareMagicMessageInternal) {
  MessageShareRequest messageShareRequest;
  messageShareRequest.sharerInfo.userId = "test_18";
  messageShareRequest.magicMessage.messageXmlContent = messageXmlContent;
  
  MessageShareResponse messageShareResponse;
  OpenMagicMessageHandler open_magic_message_handler_;
  LOG(INFO) << "ShareMagicMessageInternal...";
  if (!open_magic_message_handler_.ShareMagicMessageInternal(
          messageShareResponse, messageShareRequest)) {
    LOG(ERROR) << "ShareMagicMessageInternal error";
  }
  EXPECT_EQ(ReturnCode::SUCCESSFUL, messageShareResponse.returnCode);
  LOG(INFO) << "VideoUrl: "
    << messageShareResponse.openMagicMessage.messageContentUrl;
}
}  // namespace multimedia
}  // namespace juyou
