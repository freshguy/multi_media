// Copyright (c) 2013 Juyou Inc. All rights reserved.
// Author: wangjuntao@juyou.com (Juntao Wang)

#ifdef __cplusplus
extern "C" {
#include <magick/methods.h>
#endif

#ifdef __cplusplus
}
#endif

#include <Magick++.h>  // for Magick++
#include "file/file.h"
#include "third_party/zlib/zlib.h"
#include "juyou/base/base64/ZBase64.h"
#include "juyou/multi_media/public/open_magic_message_handler.h"
#include "juyou/multi_media/public/multimedia_transform.h"

DEFINE_string(file_transfer_server_host, "42.96.174.198",
      "The file transfer server host");
DEFINE_int32(file_transfer_server_port, 12000,
      "The file transfer server port");

/// for small remote file server
DEFINE_string(remote_file_server_host, "42.96.174.198",
      "The remote file server host");
DEFINE_int32(remote_file_server_port, 9320,
      "The remote file server port");

namespace juyou {
namespace multimedia {

OpenMagicMessageHandler::OpenMagicMessageHandler() {
  /// Reference to: http://www.lhelper.org/newblog/?tag=locksemaphoreinfo
  Magick::InitializeMagick("ShareMagicMessage Magick");
  /// Create FileTransferServiceServletClient
  file_transfer_servlet_client_ = boost::shared_ptr<base::ThriftClient<
    FileTransferServiceServletClient> >(new base::ThriftClient<
          FileTransferServiceServletClient>(FLAGS_file_transfer_server_host,
            FLAGS_file_transfer_server_port));
  CHECK(file_transfer_servlet_client_.get())
    << "Null pointer for file_transfer_servlet_client_";
  
  /// Create remote file server client
  remote_file_server_client_ = boost::shared_ptr<base::ThriftClient<
    RemoteFileHandleServletClient> >(new base::ThriftClient<
          RemoteFileHandleServletClient>(FLAGS_remote_file_server_host,
            FLAGS_remote_file_server_port));
  CHECK(remote_file_server_client_.get())
    << "Null pointer for remote_file_server_client_";

  /// Create MagicMessageHandleThread
  magicMessageHandleThread_ = new MagicMessageHandleThread(this);
  CHECK(magicMessageHandleThread_)
    << "Null pointer for magicMessageHandleThread_";
  magicMessageHandleThread_->Start();  // Start the thread
  VLOG(4) << "Start the MagicMessageHandleThread in OpenMagicMessageHandler";
}

OpenMagicMessageHandler::~OpenMagicMessageHandler() {
  if (magicMessageHandleThread_) {
    delete magicMessageHandleThread_;
  }
  magicMessageHandleThread_ = NULL;
}

bool OpenMagicMessageHandler::ShareMagicMessageInternal(
      MessageShareResponse& _return,
      const MessageShareRequest& messageShareRequest) {
  if (messageShareRequest.sharerInfo.userId.empty()) {
    LOG(ERROR) << "Empty userId for sharerInfo when ShareMagicMessage";
    _return.returnCode = ReturnCode::BAD_PARAMETER;
    return false;
  }
  if (messageShareRequest.magicMessage.messageXmlContent.empty()) {
    LOG(ERROR) << "Empty messageXmlContent when ShareMagicMessage";
    _return.returnCode = ReturnCode::BAD_PARAMETER;
    return false;
  }
  VLOG(4) << "[ShareMagicMessageInternal] messageXmlContent: "
    << messageShareRequest.magicMessage.messageXmlContent;
  
  TiXmlDocument* xmlDocument = new TiXmlDocument();
  xmlDocument->Parse(
        messageShareRequest.magicMessage.messageXmlContent.c_str());

  TiXmlPrinter printer;
  xmlDocument->Accept(&printer);
  VLOG(5) << "MessageXmlFormat output: " << printer.CStr();

  /// Extract resources' remoteFileName on tfs server
  std::map<std::string, std::string> resourcesMap;
  TiXmlElement* rootElement = xmlDocument->RootElement();
  VLOG(4) << "Message RootName: " << rootElement->Value();
  TiXmlNode* pNode = NULL;
  for (pNode = rootElement->FirstChildElement(); pNode;
        pNode = pNode->NextSiblingElement()) {
    std::string nodeName = pNode->Value();
    if (nodeName == "ftpkey") {  // for resources' xmlElement
      TiXmlElement* pChildElement = pNode->FirstChildElement();
      if (NULL == pChildElement) {
        continue;
      }
      std::string localFile, remoteFile;
      int resourceNodeCnt = 0;
      for (; pChildElement;
            pChildElement = pChildElement->NextSiblingElement()) {
        if (std::string("name") == pChildElement->Value()) {
          const char* contentPtr = pChildElement->GetText();
          if (!contentPtr) {  // if 'name' feild is NULL, return false
            LOG(ERROR) << "Empty content for 'name' feild. with "
              << "messageXmlContent: " << messageShareRequest.magicMessage.messageXmlContent;
            _return.returnCode = ReturnCode::XML_CONTENT_ERRORS;
            return false;
          }
          localFile = contentPtr;
        } else if (std::string("value") == pChildElement->Value()) {
          const char* contentPtr = pChildElement->GetText();
          if (!contentPtr) {  // if 'value' feild is NULL, return false 
            LOG(ERROR) << "Empty content for 'value' feidl. with "
              << "messageXmlContent: " << messageShareRequest.magicMessage.messageXmlContent;
            _return.returnCode = ReturnCode::XML_CONTENT_ERRORS;
            return false;
          }
          remoteFile = contentPtr;
        } else if (std::string("localvalue") == pChildElement->Value()) {
        }
        resourceNodeCnt++;
      }
      if (DEFAULT_TFSKEY_NODE_CNT == resourceNodeCnt) {  // the ftpkey's node number is 3
        resourcesMap.insert(std::pair<std::string, std::string>(
                localFile, remoteFile));
      }
    }
  }

  /// Check remote files list whether contain 'template_1.txt' file
  if (resourcesMap.find(TEMPLATE_LOCAL_NAME) == resourcesMap.end()) {
    LOG(ERROR) << "Don't contain 'template_1.txt' file in localFiles list "
      << "when ShareMagicMessage with messageXmlContent: "
      << messageShareRequest.magicMessage.messageXmlContent;
    _return.returnCode = ReturnCode::XML_CONTENT_ERRORS;
    return false;
  }

  /// Get remoteFile content from remote file server(such as TFS)
  std::map<std::string, RemoteFileInfo> remoteFileInfoMap;
  int mapCnt = 0;
  for (std::map<std::string, std::string>::iterator map_it
        = resourcesMap.begin(); map_it != resourcesMap.end();
        ++map_it) {
    VLOG(4) << ++mapCnt << "th resource. localFile -- remoteFile: "
      << map_it->first << " -- " << map_it->second;
    std::string remoteFileContent;
    try {
      file_transfer_servlet_client_.get()->GetService()->FileRead(
            remoteFileContent, map_it->second); 
    } catch(const ::apache::thrift::TException& err) {
      LOG(ERROR) << "Get remoteFileContent exception when ShareMagicMessage "
        << "with localFileName = " << map_it->first
        << ", remoteFileName = " << map_it->second
        << ", exception: " << err.what();
      _return.returnCode = ReturnCode::STORAGE_ERRORS;
      return false;
    } catch(...) {
      LOG(ERROR) << "Get remoteFileContent error when ShareMagicMessage "
        << "with localFileName = " << map_it->first
        << ", remoteFileName = " << map_it->second;
      _return.returnCode = ReturnCode::STORAGE_ERRORS;
      return false;
    }

    RemoteFileInfo remoteFileInfo;
    remoteFileInfo.remoteFileName = map_it->second;
    if (!ExtractActualRemoteFileInfo(remoteFileContent,
            remoteFileInfo.typeName,
            remoteFileInfo.origLocalFileName,
            remoteFileInfo.remoteFileContent)) {
      LOG(ERROR) << "ExtractActualRemoteFileInfo error when ShareMagicMessage";
      return false;
    }
    remoteFileInfoMap.insert(std::pair<std::string, RemoteFileInfo>(
            map_it->first, remoteFileInfo));
  }

  /// [TEST] write local file for verification
  char origWorkDir[512];
  getwd(origWorkDir);
  VLOG(4) << "CurrentDir: " << origWorkDir;
 
  std::string pathPrefix = "../res/";
  for (std::map<std::string, RemoteFileInfo>::iterator info_it
        = remoteFileInfoMap.begin();
        info_it != remoteFileInfoMap.end(); ++info_it) {
    std::string writeFileName = pathPrefix + info_it->second.origLocalFileName;
    file::File::WriteStringToFile(info_it->second.remoteFileContent,
          writeFileName);
  }

  /*
   * ******* Contruct the MagicMessageControl
   */
  MagicMessageControl* magic_message_control = new MagicMessageControl;
  /// 1. Extract message control rule
  VLOG(4) << "****** [ShareMagicMessage] Extract Message control rule ******";
  if (!magic_message_control->ExtractMessageControlRule(
        remoteFileInfoMap[TEMPLATE_LOCAL_NAME].remoteFileContent)) {
    LOG(ERROR) << "ExtractMessageControlRule error when ShareMagicMessage "
      << "with [json_remote_file_name]: " << remoteFileInfoMap[TEMPLATE_LOCAL_NAME].remoteFileName
      << ", [json_remote_file_content]: " << remoteFileInfoMap[TEMPLATE_LOCAL_NAME].remoteFileContent;
    _return.returnCode = ReturnCode::ERRORS;
    delete magic_message_control;
    magic_message_control = NULL;
    return false;
  }
  /// 2. Set message elements' contents
  VLOG(4) << "****** [ShareMagicMessage] Set Message elements' contents ******";
  if (!magic_message_control->SetMessageElementContents(
          remoteFileInfoMap)) {
    LOG(ERROR) << "SetMessageElementContents error when ShareMagicMessage";
    _return.returnCode = ReturnCode::ERRORS;
    delete magic_message_control;
    magic_message_control = NULL;
    return false;
  }

  /*
   * ****** Handle all elements by Message control rule in thread
   */
  VLOG(4) << "***** [ShareMagicMessage] Delivery the magic_message to MagicMessageHandleThread *****";
  /// 1. Upload a fake messageVideo to WebServer
  ::juyou::circles::LocalFileInfo localFileInfo;
  localFileInfo.fileName
    = Utility::GetUuid("message_video_") + ".flv";  // ".mp4";
  localFileInfo.fileContent = "fake a messageVideo";  // fake content messageVideo
  localFileInfo.decoderType = ::juyou::circles::DecoderType::NONE;
  localFileInfo.fileOwner = "magic_video";
  localFileInfo.__isset.fileOwner = true;             // set
  localFileInfo.filePrefix = "message_video_";
  localFileInfo.__isset.filePrefix = true;            // set
  localFileInfo.pathAssignMode
    = ::juyou::circles::PathAssignMode::CLIENT_ASSIGN;
  localFileInfo.__isset.pathAssignMode = true;        // set
  ::juyou::circles::RemoteFileResponse remoteFileResponse;
  remote_file_server_client_.get()->GetService()->UploadFile(
        remoteFileResponse, localFileInfo);
  if (::juyou::circles::ReturnCode::SUCCESSFUL
        != remoteFileResponse.returnCode) {
    LOG(ERROR) << "Upload video file to WebServer error when ShareMagicMessageInternal";
    _return.returnCode = ReturnCode::ERRORS;
    return false;
  }
  localFileInfo.fileContent = "";  // reset the fileContent
#if 0
  _return.openMagicMessage.messageContentUrl
    = remoteFileResponse.remoteFileUrl;           // VideoUrl
#endif
  /// Rewrite the remoteFileUrl
  _return.openMagicMessage.messageContentUrl
    = "http://42.96.174.198/USERDIR/magic_video/test.php?movie=" + localFileInfo.fileName;
  
  MagicMessageHandleInfo magicMessageHandleInfo;
  magicMessageHandleInfo.magic_message_control_ = magic_message_control;
  magicMessageHandleInfo.localFileInfo_ = localFileInfo;

  /// 2. Push the MagicMessageHandleInfo into the HandleThread queue
  magicMessagesHandleMutex_.Lock();    // Lock
  magicMessageHandleInfos_.push_back(
        magicMessageHandleInfo);
  VLOG(4) << "magicMessagesDeque size: "
    << magicMessageHandleInfos_.size();
  magicMessagesHandleMutex_.Unlock();  // Unlock
  magicMessagesHandleCondVar_.Signal();
  
  _return.returnCode = ReturnCode::SUCCESSFUL;
  return true;
}

/// for MagicMessageHandleThread
MagicMessageHandleThread::MagicMessageHandleThread(
      OpenMagicMessageHandler* openMagicMessageHandler) {
  openMagicMessageHandler_ = openMagicMessageHandler;
}

MagicMessageHandleThread::~MagicMessageHandleThread() {
}

void MagicMessageHandleThread::Run() {
  while (true) {
    MagicMessageHandleInfo magicMessageHandleInfo;
    bool isItemValid = false;
    
    openMagicMessageHandler_->get_magic_message_handle_mutex().Lock();  // Lock
    
    while (openMagicMessageHandler_->get_magic_message_infos().size() == 0) {
      openMagicMessageHandler_->get_magic_message_handle_condvar().Wait(
            &openMagicMessageHandler_->get_magic_message_handle_mutex());
      isItemValid = false;
    }
    magicMessageHandleInfo.magic_message_control_
      = openMagicMessageHandler_->get_magic_message_infos().front().magic_message_control_;
    magicMessageHandleInfo.localFileInfo_
      = openMagicMessageHandler_->get_magic_message_infos().front().localFileInfo_;

    /// pop this message from queue head
    openMagicMessageHandler_->get_magic_message_infos().pop_front();
    isItemValid = true;
    openMagicMessageHandler_->get_magic_message_handle_mutex().Unlock();  // Unlock
    
    if (isItemValid) {
      /// Contert MagicMessage to Video
      base::TimeTicks startTicks = base::TimeTicks::HighResNow();
      if (!magicMessageHandleInfo.magic_message_control_->MagicMessage2Video(
              magicMessageHandleInfo.localFileInfo_.fileContent)) {
        LOG(ERROR) << "MagicMessage2Video error in HandleThread with "
          << "localFileName: "
          << magicMessageHandleInfo.localFileInfo_.fileName;
        /// Destroy the magic_message_control
        delete magicMessageHandleInfo.magic_message_control_;
        magicMessageHandleInfo.magic_message_control_ = NULL;
        continue;
      }
      base::TimeTicks endTicks = base::TimeTicks::HighResNow();
      VLOG(4) << "MagicMessage2Video consume: "
        << (endTicks - startTicks).InMilliseconds() << "ms";

      /// Upload the actual magicMessage video to WebServer
      ::juyou::circles::RemoteFileResponse remoteFileResponse;
      openMagicMessageHandler_->get_remote_file_handle_servlet_client()->GetService()->UploadFile(
            remoteFileResponse, magicMessageHandleInfo.localFileInfo_);
      if (::juyou::circles::ReturnCode::SUCCESSFUL
            != remoteFileResponse.returnCode) {
        LOG(ERROR) << "Upload actual messageVideo to WebServer error"
          << " in HandleThread with localFileName: "
          << magicMessageHandleInfo.localFileInfo_.fileName;
        /// Destroy the magic_message_control
        delete magicMessageHandleInfo.magic_message_control_;
        magicMessageHandleInfo.magic_message_control_ = NULL;
        continue;
      }
      /// Destroy the magic_message_control
      delete magicMessageHandleInfo.magic_message_control_;
      magicMessageHandleInfo.magic_message_control_ = NULL;
    }
  }
}
}  // namespace multimedia
}  // namespace juyou
