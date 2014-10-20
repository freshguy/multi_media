// Copyright (c) 2013 Juyou Inc. All rights reserved.
// Author: wangjuntao@juyou.com (Juntao Wang)

#include <string>
#include <vector>
#include "base/logging.h"
#include "base/thrift.h"
#include "base/flags.h"
#include "file/file.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/tinyxml/tinyxml.h"
#include "juyou/multi_media/proto/gen-cpp/OpenMagicMessageServlet.h"

#define SERVER_HOST "219.232.227.247"  // "115.28.40.14"
#define SERVER_PORT 9627

DEFINE_string(server_host, "115.28.40.14",  // "219.232.227.247",
      "The server host");
DEFINE_int32(server_port, 9627,
      "The server port");

// std::string messageXmlContent = "<?xml version='1.0' encoding='UTF-8' standalone='yes' ?><story><cachepath>2013_11_05__02_33_3882f97e4f-bc4d-42fb-a3cd-64d665a23680_template_1</cachepath><cachename></cachename><x>-1</x><y>-1</y><width>-1</width><height>-1</height><startscriptname>PROCESS</startscriptname><ftpkey><name>template_1.txt</name><value>T1_tETBy_T1RCvBVdK</value><localvalue></localvalue></ftpkey><ftpkey><name>a0b7128a-b749-4edd-9179-7a74ad93c448.Ogg</name><value>T1pyhTBbYj1RCvBVdK</value><localvalue></localvalue></ftpkey><ftpkey><name>a0b7128a-b749-4edd-9179-7a74ad93c448.gif</name><value>T1pRhTBb_j1RCvBVdK</value><localvalue></localvalue></ftpkey></story>";

// std::string messageXmlContent = "<story> <cachepath>T1_yETByYT1RCvBVdK</cachepath><cachename></cachename><x>-1</x><y>-1</y><width>-1</width><height>-1</height><startscriptname>PROCESS</startscriptname><ftpkey><name>e2d59b61-87f5-4c72-a4ee-03eff5b6100c_reference.zip</name><value>T1_aETByYT1RCvBVdK</value><localvalue></localvalue></ftpkey><ftpkey><name>e2d59b61-87f5-4c72-a4ee-03eff5b6100c_ele1.png</name><value>T1FRhTByCQ1RCvBVdK</value><localvalue></localvalue></ftpkey><ftpkey><name>e2d59b61-87f5-4c72-a4ee-03eff5b6100c_template.png</name><value>T1_RETByYT1RCvBVdK</value><localvalue></localvalue></ftpkey><ftpkey><name>e2d59b61-87f5-4c72-a4ee-03eff5b6100c_music.ogg</name><value>T15tETByYT1RCvBVdK</value><localvalue></localvalue></ftpkey><ftpkey><name>e2d59b61-87f5-4c72-a4ee-03eff5b6100c_mask.png</name><value>T15yETByYT1RCvBVdK</value><localvalue></localvalue></ftpkey><ftpkey><name>template_1.txt</name><value>T1_yETByYT1RCvBVdK</value><localvalue></localvalue></ftpkey><ftpkey><name>e2d59b61-87f5-4c72-a4ee-03eff5b6100c_user.zip</name><value>T1FyhTBybQ1RCvBVdK</value><localvalue></localvalue></ftpkey></story>";

// std::string messageXmlContent = "<story> <cachepath>T1pthTBsKj1RCvBVdK1</cachepath><cachename></cachename><x>-1</x><y>-1</y><width>-1</width><height>-1</height><startscriptname>PROCESS</startscriptname><ftpkey><name>ZIPGODPEN_44235d23-6f59-4860-9249-5ab8f5156630.zip</name><value>T1XRETBXKT1RCvBVdK</value><localvalue></localvalue></ftpkey><ftpkey><name>317916ca-ab01-4f75-b56b-06f94dbd226b_godpen.zip</name><value>T1XRETB4VT1RCvBVdK</value><localvalue></localvalue></ftpkey><ftpkey><name>eed06641-50e8-4deb-8408-e8f9e8c45408.ogg</name><value>T1FahTBsYj1RCvBVdK</value><localvalue></localvalue></ftpkey><ftpkey><name>template_1.txt</name><value>T1XyETB4VT1RCvBVdK</value><localvalue></localvalue></ftpkey><ftpkey><name>842825f7-6114-4d3b-9248-61db86758b5b.png</name><value>T1FRhTByAQ1RCvBVdK</value><localvalue></localvalue></ftpkey></story>";

// std::string messageXmlContent = "<story> <cachepath>T1pthTByCQ1RCvBVdK</cachepath><cachename></cachename><x>-1</x><y>-1</y><width>-1</width><height>-1</height><startscriptname>PROCESS</startscriptname><ftpkey><name>6716737ccf414abca88e51820a7ffb42_frame.png</name><value>T1pyhTBy_Q1RCvBVdK</value><localvalue></localvalue></ftpkey><ftpkey><name>template_1.txt</name><value>T1pthTByCQ1RCvBVdK</value><localvalue></localvalue></ftpkey><ftpkey><name>6716737ccf414abca88e51820a7ffb42_user_1.zip</name><value>T1FthTBsZj1RCvBVdK</value><localvalue></localvalue></ftpkey><ftpkey><name>6716737ccf414abca88e51820a7ffb42_user_2.zip</name><value>T1RahTBsxj1RCvBVdK</value><localvalue></localvalue></ftpkey></story>";

// std::string messageXmlContent = "<?xml version='1.0' encoding='utf8'?><story> <cachepath>T1XRETBydT1RCvBVdK</cachepath><cachename></cachename><x>-1</x><y>-1</y><width>-1</width><height>-1</height><startscriptname>PROCESS</startscriptname><ftpkey><name>804fd3f9-e1d0-48b7-b1e3-042e9b6bd211.ogg</name><value>T12ahTBQKj1RCvBVdK</value><localvalue></localvalue></ftpkey><ftpkey><name>2013_09_18__22_52_07136be89c-8fe6-4d8b-97d2-19f981b323d4.zip</name><value>T1dyhTBQVj1RCvBVdK</value><localvalue></localvalue></ftpkey><ftpkey><name>template_1.txt</name><value>T1TyETByVT1RCvBVdK</value><localvalue></localvalue></ftpkey></story>";

/// TEXT
std::string messageXmlContent = "<?xml version='1.0' encoding='utf8'?><story> <cachepath>T1XRETBydT1RCvBVdK</cachepath><cachename></cachename><x>-1</x><y>-1</y><width>-1</width><height>-1</height><startscriptname>PROCESS</startscriptname><ftpkey><name>804fd3f9-e1d0-48b7-b1e3-042e9b6bd211.ogg</name><value>T12ahTBQKj1RCvBVdK</value><localvalue></localvalue></ftpkey><ftpkey><name>2013_09_18__22_52_07136be89c-8fe6-4d8b-97d2-19f981b323d4.zip</name><value>T1dyhTBQVj1RCvBVdK</value><localvalue></localvalue></ftpkey><ftpkey><name>template_1.txt</name><value>T1TyETBybT1RCvBVdK</value><localvalue></localvalue></ftpkey></story>";

// std::string messageXmlContent = "<?xml version='1.0' encoding='utf8'?><story><cachepath>2014_01_01__00_35_5278ad9bcb-7ea4-4630-a60a-2f7d81823d53_template_1</cachepath><cachename></cachename><x>-1</x><y>-1</y><width>-1</width><height>-1</height><startscriptname>PROCESS</startscriptname><ftpkey><name>template_1.txt</name><value>T1FyhTByLQ1RCvBVdK</value><localvalue>&lt;![CDATA[]]&gt;</localvalue></ftpkey><ftpkey><name>83e10a3fea324c48a355dd8f9609233213.png</name><value>T1FthTBsYj1RCvBVdK</value><localvalue>&lt;![CDATA[]]&gt;</localvalue></ftpkey><ftpkey><name>MP4_2014_01_01__00_35_16cf967f96-3c88-44f2-b8b8-508d1c2a7caa.zip</name><value>T1RahTBsYj1RCvBVdK</value><localvalue>&lt;![CDATA[]]&gt;</localvalue></ftpkey></story>";

// std::string messageXmlContent = "<?xml version='1.0' encoding='utf8'?><story><cachepath>2014_01_01__12_06_20652dd495-1c76-424f-ba70-3f7f486f6561_template_1</cachepath><cachename></cachename><x>-1</x><y>-1</y><width>-1</width><height>-1</height><startscriptname>PROCESS</startscriptname><ftpkey><name>template_1.txt</name><value>T1KRETB5YT1RCvBVdK</value><localvalue>&lt;![CDATA[]]&gt;</localvalue></ftpkey><ftpkey><name>2014_01_01__12_05_4697287ccb-909a-4ee1-a7da-01dc3544341b.jpg</name><value>T1KyETB5ZT1RCvBVdK</value><localvalue>&lt;![CDATA[]]&gt;</localvalue></ftpkey><ftpkey><name>MP4_2014_01_01__12_06_02ce8aa71f-c21d-48c4-9dfb-6e7ea6656582.zip</name><value>T1KtETBXYT1RCvBVdK</value><localvalue>&lt;![CDATA[]]&gt;</localvalue></ftpkey></story>";

// std::string messageXmlContent = "<?xml version='1.0' encoding='utf8'?><story><cachepath>2014_01_04__22_20_17ec55a461-413e-4468-95cb-91c0791daa7b_template_1</cachepath><cachename></cachename><x>-1</x><y>-1</y><width>-1</width><height>-1</height><startscriptname>PROCESS</startscriptname><ftpkey><name>template_1.txt</name><value>T1jRETBydT1RCvBVdK</value><localvalue>&lt;![CDATA[]]&gt;</localvalue></ftpkey><ftpkey><name>2014_01_04__22_18_29500d9c80-a89f-469e-b2f2-4cd431accbdf.jpg</name><value>T1jyETBydT1RCvBVdK</value><localvalue>&lt;![CDATA[]]&gt;</localvalue></ftpkey><ftpkey><name>MP4_2014_01_04__22_18_4759b4800b-5319-4b2f-aea4-5da874e1a207.zip</name><value>T1jtETByZT1RCvBVdK</value><localvalue>&lt;![CDATA[]]&gt;</localvalue></ftpkey><ftpkey><name>MP4_2014_01_04__22_19_05551e9a2b-3d16-4f32-8c8d-9c68767f57c2.zip</name><value>T1QaETByxT1RCvBVdK</value><localvalue>&lt;![CDATA[]]&gt;</localvalue></ftpkey><ftpkey><name>ed6aaa71-0306-4182-861d-deefc3fa6749.gif</name><value>T1XyETBXVT1RCvBVdK</value><localvalue></localvalue></ftpkey></story>";

// std::string messageXmlContent = "<?xml version='1.0' encoding='utf8'?><story><cachepath>2014_01_04__22_38_14ade320bf-33c7-416c-821f-77c10cf72f78_template_1</cachepath><cachename></cachename><x>-1</x><y>-1</y><width>-1</width><height>-1</height><startscriptname>PROCESS</startscriptname><ftpkey><name>template_1.txt</name><value>T1QRETByxT1RCvBVdK</value><localvalue>&lt;![CDATA[]]&gt;</localvalue></ftpkey><ftpkey><name>2014_01_04__22_35_51367ac273-9f9f-451c-b3ac-47b634a035f9.jpg</name><value>T1QyETByxT1RCvBVdK</value><localvalue>&lt;![CDATA[]]&gt;</localvalue></ftpkey><ftpkey><name>MP4_2014_01_04__22_36_128fda0123-4f5a-4350-bde1-c40557c7762e.zip</name><value>T1QtETByxT1RCvBVdK</value><localvalue>&lt;![CDATA[]]&gt;</localvalue></ftpkey><ftpkey><name>6c7efcff-b93e-4793-8b14-e1be357d4976.Ogg</name><value>T1XtETB4YT1RCvBVdK</value><localvalue></localvalue></ftpkey><ftpkey><name>6c7efcff-b93e-4793-8b14-e1be357d4976.gif</name><value>T1XyETB4CT1RCvBVdK</value><localvalue></localvalue></ftpkey></story>";

/// Video flip-left
// std::string messageXmlContent = "<?xml version='1.0' encoding='utf8'?><story><cachepath>2014_01_04__22_46_40bd9c8053-cbe4-4ba4-9eb4-6e46c390b728_template_1</cachepath><cachename></cachename><x>-1</x><y>-1</y><width>-1</width><height>-1</height><startscriptname>PROCESS</startscriptname><ftpkey><name>template_1.txt</name><value>T19aJTByxT1RCvBVdK</value><localvalue>&lt;![CDATA[]]&gt;</localvalue></ftpkey><ftpkey><name>2014_01_04__22_43_14b3d99986-d0a8-4d77-b38a-3a6f3a511dc5.jpg</name><value>T19RJTByJT1RCvBVdK</value><localvalue>&lt;![CDATA[]]&gt;</localvalue></ftpkey><ftpkey><name>20140104224348.mp4</name><value>T19yJTByJT1RCvBVdK</value><localvalue>&lt;![CDATA[]]&gt;</localvalue></ftpkey><ftpkey><name>fd01d22c-352f-4705-9ee9-7adea78557b4.gif</name><value>T1FahTBsbj1RCvBVdK</value><localvalue></localvalue></ftpkey><ftpkey><name>fd01d22c-352f-4705-9ee9-7adea78557b4.gif</name><value>T1FahTBsbj1RCvBVdK</value><localvalue></localvalue></ftpkey><ftpkey><name>2014_01_04__22_44_09e1ebfc36-c6a8-441b-87a6-c5075d872a13.ogg</name><value>T19tJTByJT1RCvBVdK</value><localvalue>&lt;![CDATA[]]&gt;</localvalue></ftpkey></story>";

// std::string messageXmlContent = "<?xml version='1.0' encoding='utf8'?><story><cachepath>2014_01_06__12_16_1057c6871d-3b2e-4e4a-a0bf-634dd38b94dd_template_1</cachepath><cachename/><x>-1</x><y>-1</y><width>-1</width><height>-1</height><startscriptname>PROCESS</startscriptname><ftpkey><name>template_1.txt</name><value>T1RthTBsJj1RCvBVdK</value><localvalue/></ftpkey><ftpkey><name>2014_01_06__12_14_064522b1b0-827a-4299-8baf-f28d56ca1266.jpg</name><value>T1dahTBsEj1RCvBVdK</value><localvalue/></ftpkey><ftpkey><name>2014_01_06__12_15_07409c3f41-6722-4eb4-b3a2-3b95a12dc3c4.zip</name><value>T1dRhTBsJj1RCvBVdK</value><localvalue/></ftpkey><ftpkey><name>2014_01_06__12_15_252cced76d-d82c-48e9-94f0-d5546023fd2b.ogg</name><value>T1fRJTByJT1RCvBVdK</value><localvalue/></ftpkey></story>";

//std::string messageXmlContent = "<?xml version='1.0' encoding='utf8'?><story><cachepath>2014_01_07__17_56_11cc4aed75-1de1-4119-92bd-959b87801dbf_template_1</cachepath><cachename/><x>-1</x><y>-1</y><width>-1</width><height>-1</height><startscriptname>PROCESS</startscriptname><ftpkey><name>template_1.txt</name><value>T1KtETBXCT1RCvBVdK</value><localvalue/></ftpkey><ftpkey><name>2014_01_07__17_54_11f8536feb-5a1e-4983-9221-5ade2a516986.jpg</name><value>T17aETBXKT1RCvBVdK</value><localvalue/></ftpkey><ftpkey><name>2014_01_07__17_55_29d2244336-6312-4124-8390-0cc6ceb60634.zip</name><value>T17RETBXDT1RCvBVdK</value><localvalue/></ftpkey><ftpkey><name>2014_01_07__17_55_52eea0210a-2668-45a0-b64a-4b612c7a7d88.mp3</name><value>T1dyhTBsJj1RCvBVdK</value><localvalue/></ftpkey></story>";

using boost::shared_ptr;

namespace juyou {
namespace multimedia {

TEST(OpenMagicMessageServletHandler, ShareMagicMessage_test) {
  shared_ptr<base::ThriftClient<OpenMagicMessageServletClient> >
    open_magic_message_server_client_ = shared_ptr<
    base::ThriftClient<OpenMagicMessageServletClient> >
    (new base::ThriftClient<OpenMagicMessageServletClient>(FLAGS_server_host,
                                                           FLAGS_server_port));

  MessageShareRequest messageShareRequest;
  messageShareRequest.sharerInfo.userId = "test_18";
  messageShareRequest.magicMessage.messageXmlContent = messageXmlContent;
  TiXmlDocument* pDoc = new TiXmlDocument();
  pDoc->Parse(messageXmlContent.c_str());
  LOG(INFO) << "messageXmlContent format output: ";
  pDoc->Print();

  MessageShareResponse messageShareResponse;
  open_magic_message_server_client_.get()->GetService()->ShareMagicMessage(
        messageShareResponse, messageShareRequest);
  EXPECT_EQ(ReturnCode::SUCCESSFUL, messageShareResponse.returnCode);
  LOG(INFO) << "openMagicMessageUrl: "
    << messageShareResponse.openMagicMessage.messageContentUrl;
}
}  // namespace multimedia
}  // namespace juyou
