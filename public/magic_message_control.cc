// Copyright (c) 2013 Juyou Inc. All rights reserved.
// Author: wangjuntao@juyou.com (Juntao Wang)

#include <map>
#include <algorithm>
#include "file/file.h"
#include "juyou/circles/public/utility.h"
#include "juyou/multi_media/public/magic_message_control.h"
#include "juyou/multi_media/msg_action/msg_action_play.h"
#include "juyou/multi_media/msg_action/msg_action_visibility.h"
#include "juyou/multi_media/msg_action/msg_action_rotate.h"
#include "juyou/multi_media/msg_action/msg_action_scale.h"
#include "juyou/multi_media/msg_action/msg_action_translate.h"
#include "juyou/multi_media/msg_action/msg_action_text_animate.h"
#include "juyou/multi_media/media/multimedia_handler.h"

DEFINE_string(multimedia_file_dir, "/tmp",
      "The multi-media file default parent dir");
DEFINE_string(default_video_name, "magic_message.flv",  // "magic_message.mp4",
      "The default magic_message file name");

namespace juyou {
namespace multimedia {

MagicMessageControl::MagicMessageControl() {
}

MagicMessageControl::~MagicMessageControl() {
}
 
bool MagicMessageControl::ExtractMessageControlRule(
      const std::string& json_str) {
  if (json_str.empty()) {
    LOG(ERROR) << "Empty json_str when ExtractMessageControlRule";
    return false;
  }
  /// Parse std::string --> Json::Value
  Json::Reader reader;
  Json::Value control_json_;
  if (!reader.parse(json_str, control_json_)) {
    LOG(ERROR) << "Parse Json string failed when ExtractMessageControlRule"
      << " with json_str: " << json_str
      << ". [Json::Reader::getFormatedErrorMessages]: "
      << reader.getFormatedErrorMessages();
    return false;
  }

  /// Parse the Json::Value --> MagicMessageControlRule
  if (!control_json_.isMember("screenWidth")) {
    LOG(ERROR) << "No screenWidth field in control_json";
    return false;
  }
  magic_message_control_rule_.screenWidth_
    = control_json_["screenWidth"].asInt();   // screenWidth
  if (!control_json_.isMember("screenHeight")) {
    LOG(ERROR) << "No screenHeight field in control_json";
    return false;
  }
  magic_message_control_rule_.screenHeight_
    = control_json_["screenHeight"].asInt();  // screenHeight
  if (!control_json_.isMember("baseUrl")) {
    LOG(ERROR) << "No baseUrl field in control_json";
    return false;
  }
  magic_message_control_rule_.baseUrl_
    = control_json_["baseUrl"].asString();    // baseUrl
  if (!control_json_.isMember("duration")) {
    LOG(ERROR) << "No duration field in control_json";
    return false;
  }
  magic_message_control_rule_.duration_
    = control_json_["duration"].asInt();      // duration
  if (!control_json_.isMember("elements")) {
    LOG(ERROR) << "No elements field in control_json";
    return false;
  }
  VLOG(4) << "$$$$$$$$ duration: " << magic_message_control_rule_.duration_;
  /// Invoke must after screenWidth_ and screenHeight_ initialize
  if (!ExtractElement(control_json_["elements"])) {  // elements
    LOG(ERROR) << "ExtractElement error when ExtractMessageControlRule";
    return false;
  }
  VLOG(4) << " ********* ExtractMessageControlRule DUMP start ********";
  VLOG(4) << "screenWidth_: " << magic_message_control_rule_.screenWidth_;
  VLOG(4) << "screenHeight_: " << magic_message_control_rule_.screenHeight_;
  VLOG(4) << "baseUrl_: " << magic_message_control_rule_.baseUrl_;
  VLOG(4) << "duration_: " << magic_message_control_rule_.duration_;
  VLOG(4) << "messageElements_.size(): "
    << magic_message_control_rule_.messageElements_.size();
  VLOG(4) << " ********* ExtractMessageControlRule DUMP end ********";
  return true;
}

bool MagicMessageControl::SetMessageElementContents(
      const std::map<std::string, RemoteFileInfo>& elementNameContentMap) {
  if (elementNameContentMap.empty()) {
    LOG(ERROR)
      << "Empty elementNameContentMap when SetMessageElementContents";
    return false;
  }
  std::vector<MessageElement*>& messageElements
    = magic_message_control_rule_.messageElements_;
  /// Check whether contain all the element contents
  for (std::vector<MessageElement*>::iterator e_it
        = messageElements.begin();
        e_it != messageElements.end(); ++e_it) {
    if (elementNameContentMap.find((*e_it)->url_)
          == elementNameContentMap.end()
          && (*e_it)->elementType_ != TEXT) {
      LOG(ERROR) << "No exists " << (*e_it)->url_
        << " element file in elementNameContentMap when SetMessageElementContents";
      return false;
    }
  }
  /// Set message elements' content
  std::vector<MessageElement*> videoMessageElements;
  for (std::vector<MessageElement*>::iterator e_it
        = messageElements.begin();
        e_it != messageElements.end(); ++e_it) {
    if (TEXT != (*e_it)->elementType_) {  // Not TEXT
      std::string elementContent
        = elementNameContentMap.find((*e_it)->url_)->second.remoteFileContent;
      if (!(*e_it)->SetElementContent(elementContent,
              (*e_it)->url_)) {
        LOG(ERROR) << "SetElementContent error when SetMessageElementContents "
          << "with elementName = " << (*e_it)->url_
          << ", elementType = " << (*e_it)->elementType_;
        return false;
      }
    } else {  // TEXT
      if (!(*e_it)->SetElementContent((*e_it)->text_,
              (*e_it)->url_)) {
        LOG(ERROR) << "SetElementContent(TEXT) error when SetMessageElementContents "
          << "with elementName = " << (*e_it)->url_
          << ", elementType = " << (*e_it)->elementType_;
        return false;
      }
    }
    if (VIDEO == (*e_it)->elementType_) {
      videoMessageElements.push_back(*e_it);
    }
  }

  /*
   * IMPORTANT: Use the VideoInternalAudio fake MessageElement,
   * whoes elementType_ = ElementType::AUDIO_MEDIAPLAYER (12.18)
   */
  for (std::vector<MessageElement*>::iterator e_it
        = videoMessageElements.begin();
        e_it != videoMessageElements.end(); ++e_it) {
    VLOG(4) << "Fake AUDIO_MEDIAPLAYER MessageElement for VIDEOUrl: "
      << (*e_it)->url_;
    MessageElement* messageElement = new MessageElement(
          magic_message_control_rule_.screenWidth_,
          magic_message_control_rule_.screenHeight_);
    messageElement->id_ = (*e_it)->id_ + "_videoInternalAudio";
    messageElement->elementType_ = AUDIO_MEDIAPLAYER;
    /// Set elementContent
    AudioElementContent* audioElementPtr = new AudioElementContent;
    if (!audioElementPtr->DecodeElementContent(
        ((VideoElementContent*)(*e_it)->elementContent_)->videoInternalAudioInfo_.audioContent_,
        ((VideoElementContent*)(*e_it)->elementContent_)->videoInternalAudioInfo_.audioUrlName_)) {
      LOG(ERROR) << "Fake videoInternalAudio MessageElement error for "
        << "videoUrlName = " << (*e_it)->url_;
      return false;
    }
    messageElement->elementContent_ = audioElementPtr;
    messageElement->url_
      = ((VideoElementContent*)(*e_it)->elementContent_)->videoInternalAudioInfo_.audioUrlName_;
    messageElement->isReturn_ = (*e_it)->isReturn_;
    messageElement->x_ = (*e_it)->x_;
    messageElement->y_ = (*e_it)->y_;
    messageElement->w_ = (*e_it)->w_;
    messageElement->h_ = (*e_it)->h_;
    messageElement->scale_ = (*e_it)->scale_;
    messageElement->degree_ = (*e_it)->degree_;
    messageElement->visible_ = (*e_it)->visible_;
    messageElement->volume_ = (*e_it)->volume_;

    MsgAction* videoPlayAction = NULL;
    for (std::vector<MsgAction*>::iterator it
          = (*e_it)->actions_.begin();
          it != (*e_it)->actions_.end(); ++it) {
      if (PLAY == (*it)->actionType_) {
        videoPlayAction = *it;
        break;
      }
    }
    if (!videoPlayAction) {
      LOG(ERROR) << "No PLAY action in this video: "
        << (*e_it)->url_;
      return false;
    }

    MsgAction* msgAction = new MsgActionPlay;
    msgAction->SetActionType();
    msgAction->startTime_ = videoPlayAction->startTime_;
    msgAction->duration_ = videoPlayAction->duration_;
    VLOG(4) << "######### msgActionType: " << msgAction->actionType_
      << ", startTime: " << msgAction->startTime_
      << ", duration: " << msgAction->duration_;
    messageElement->actions_.push_back(msgAction);

    /// Insert this fake AUDIO_MEDIAPLAY MessageElement
    messageElements.push_back(messageElement);
  }
  return true;
}

bool MagicMessageControl::AudioSynthesize(const std::string& audioFileDir,
      std::map<MessageElement*, AudioHandledInfo>& inputAudioFragmentInfo,
      AudioHandledInfo& handledAudioInfo) {
  VLOG(4) << "audioFileDir: " << audioFileDir
    << ", inputAudioSize: " << inputAudioFragmentInfo.size();
  int handledCnt = 0;
  for (std::map<MessageElement*, AudioHandledInfo>::iterator a_it
        = inputAudioFragmentInfo.begin();
        a_it != inputAudioFragmentInfo.end(); ++a_it) {
    if (1 != a_it->first->actions_.size()) {
      VLOG(4) << "actions.size = " << a_it->first->actions_.size();
      continue;
    }
    if (a_it->first->actions_[0]->actionType_ != PLAY) {
      VLOG(4) << "actions[0]->actionType_ = " << a_it->first->actions_[0]->actionType_;
      continue;  // Skip the no have only 'PLAY' action
    }
    /// 1. Convert to WAV format for sox
    std::string convertFileName
      = audioFileDir + "/" + a_it->second.origAudioFileName + "_convert.wav";
    std::string convertCommand;
    if (AMR == a_it->second.origAudioFormat) {
      convertCommand += "ffmpeg -i ";
      convertCommand += a_it->second.origAudioFilePath;
      convertCommand += " -ar 44100 ";
      convertCommand += convertFileName;
    } else if (OGG == a_it->second.origAudioFormat) {  // OK
      convertCommand += "ffmpeg -i ";
      convertCommand += a_it->second.origAudioFilePath;
      convertCommand += " -ar 44100 ";
      convertCommand += convertFileName;
    } else if (MP3 == a_it->second.origAudioFormat) {
      convertCommand += "ffmpeg -i ";
      convertCommand += a_it->second.origAudioFilePath;
      convertCommand += " -f wav -ar 44100 ";
      convertCommand += convertFileName;
    } else if (WAV == a_it->second.origAudioFormat) {
      convertCommand += "ffmpeg -i ";
      convertCommand += a_it->second.origAudioFilePath;
      convertCommand += " -f wav -ar 44100 ";
      convertCommand += convertFileName;
    }
#if 0
    if (WAV != a_it->second.origAudioFormat) {
      if (system(convertCommand.c_str()) > 0) {
        VLOG(4) << "Convert to wav for sox successfully";
      }
    } else {  // rename
      int flag = rename(a_it->second.origAudioFilePath.c_str(),
            convertFileName.c_str());
      if (0 != flag) {  // Successfully, return 0
        LOG(ERROR) << "rename error when AudioSynthesize "
          << "with origAudioFilePath = " << a_it->second.origAudioFilePath
          << ", convertFileName = " << convertFileName;
        return false;
      }
    }
#endif

    if (system(convertCommand.c_str()) > 0) {
      VLOG(4) << "Convert to wav for sox successfully";
    }

    /// 2. Pad a fragment silence for this audio, if the audio's startTime_ != 0
    std::string padFileName
      = audioFileDir + "/" + a_it->second.origAudioFileName + "_headPad.wav";
    int startTime = a_it->first->actions_[0]->startTime_;
    int duration = a_it->first->actions_[0]->duration_;
    if (startTime > 0) {
      std::string padCommand = "sox " + convertFileName + " ";
      padCommand += padFileName;
      padCommand += " pad ";
      padCommand += DoubleToString((double)startTime / 1000);
      if (system(padCommand.c_str()) > 0) {
        VLOG(4) << "Pad silence audio successfully";
      }
    } else {  // mv convertFileName padFileName
      int flag = rename(convertFileName.c_str(), padFileName.c_str());
      if (0 != flag) {  // Successfully, return 0
        LOG(ERROR) << "rename error when AudioSynthesize "
          << "with converFileName = " << convertFileName
          << ", padFileName = " << padFileName;
        return false;
      }
    }

    /// 3. Trim the PadFileName's end by 'startTime + duration'
    /// ffmpeg -i mp32wav.wav -t 00:00:03.05 -c copy longer3l12.wav
    std::string trimEndFileName
      = audioFileDir + "/" + a_it->second.origAudioFileName + "_trimEnd.wav";
    std::string trimEndCommand;
    if (startTime + duration > magic_message_control_rule_.duration_) {
      trimEndCommand += "ffmpeg -i ";
      trimEndCommand += padFileName;
      trimEndCommand += " -t ";
      trimEndCommand += DoubleToString((double)magic_message_control_rule_.duration_ / 1000);
      trimEndCommand += " -c copy ";
      trimEndCommand += trimEndFileName;
    } else {
      trimEndCommand += "ffmpeg -i ";
      trimEndCommand += padFileName;
      trimEndCommand += " -t ";
      trimEndCommand += DoubleToString((double)(startTime + duration) / 1000);
      trimEndCommand += " -c copy ";
      trimEndCommand += trimEndFileName;
    } 
    if (system(trimEndCommand.c_str()) > 0) {
      VLOG(4) << "trimEnd audio file successfully";
    }

    a_it->second.handledAudioFilePath = trimEndFileName;
    a_it->second.handledAudioFormat = WAV;
    a_it->second.isHandled_ = true;   // handled
    handledCnt++;
  }

  if (handledCnt < 1) {
    LOG(ERROR) << "handled audio count should not less than one";
    return false;
  }

  /// 4. Merged all the audioFiles if necessary
  std::string mergedFileName
    = audioFileDir + "/" + "allaudio_merged.wav";
  if (handledCnt == 1) {
    /// Have only one handled audio, rename it to 'mergedFileName'
    std::string onlyOneHandledAudio;
    for (std::map<MessageElement*, AudioHandledInfo>::iterator a_it
          = inputAudioFragmentInfo.begin();
          a_it != inputAudioFragmentInfo.end(); ++a_it) {
      if (a_it->second.isHandled_) {
        onlyOneHandledAudio = a_it->second.handledAudioFilePath;
        break;
      }
    }
    int flag = rename(onlyOneHandledAudio.c_str(), mergedFileName.c_str());
    if (0 != flag) {  // Successfully, return 0
      LOG(ERROR) << "rename error when merge state in AudioSynthesize "
        << "with onlyOneHandledAudio = " << onlyOneHandledAudio
        << ", mergedFileName = " << mergedFileName;
      return false;
    }
  } else {  // handledCnt > 1
    /// 4. Merged all the audioFiles which isHandled_ = true
    std::string mergeAudioCommand = "sox -m ";
    for (std::map<MessageElement*, AudioHandledInfo>::iterator a_it
          = inputAudioFragmentInfo.begin();
          a_it != inputAudioFragmentInfo.end(); ++a_it) {
      if (a_it->second.isHandled_) {
        mergeAudioCommand += a_it->second.handledAudioFilePath;
        mergeAudioCommand += " ";
      }
    }
    mergeAudioCommand += mergedFileName;
    VLOG(4) << "mergedAudioCommand: " << mergeAudioCommand;
    
    if (system(mergeAudioCommand.c_str()) > 0) {
      VLOG(4) << "mergeAudio audio successfully";
    }
  }
  
  /// 5. Convert format from WAV to MP3
  ///  ffmpeg -i mp32wav.wav -acodec libmp3lame wav2mp3.mp3
  ///  ffmpeg -i mp32wav.wav -f mp3 -acodec libmp3lame -y wav2mp3.mp3
  std::string formatConvertFileName
      = audioFileDir + "/" + "synthesize_final.mp3";
  
  std::string formatConvertCommand = "ffmpeg -i ";
  formatConvertCommand += mergedFileName;
  formatConvertCommand += " -acodec libmp3lame ";
  formatConvertCommand += formatConvertFileName;

  if (system(formatConvertCommand.c_str()) > 0) {
    VLOG(4) << "Audio systhesize successfully";
  }
  /// End
  handledAudioInfo.handledAudioFilePath = formatConvertFileName;
  handledAudioInfo.isHandled_ = true;
  return true;
}

bool MagicMessageControl::MagicMessage2Video(
      std::string& magicMessageVideoContent) {
  if (magic_message_control_rule_.screenWidth_ == 0
        || magic_message_control_rule_.screenHeight_ == 0
        || magic_message_control_rule_.duration_ == 0) {
    LOG(ERROR) << "Invalid screenWidth_, screenHeight_ or duration_ "
      << "when MagicMessage2Video with "
      << "screenWidth_ = " << magic_message_control_rule_.screenWidth_
      << ", screenHeight_ = " << magic_message_control_rule_.screenHeight_
      << ", duration_ = " << magic_message_control_rule_.duration_;
    magicMessageVideoContent = "";
    return false;
  }

  /// Get the number of the non-multimedia(such as audio, video etc.) elements
  int nonMultimediaCnt = 0;
  
  /// Create multi-media file dir
  std::string multiMediaFileDir
    = FLAGS_multimedia_file_dir + "/" + Utility::GetUuid("MEDIA_");
  if (!file::File::CreateDir(multiMediaFileDir, 0777)) {
    LOG(ERROR) << "CreateDir error when MagicMessage2Video with "
      << "multiMediaFileDir = " << multiMediaFileDir;
    return false;
  }

  std::map<MessageElement*, AudioHandledInfo> elementAudioHandledMap;
  for (std::vector<MessageElement*>::iterator m_it =
        magic_message_control_rule_.messageElements_.begin();
        m_it != magic_message_control_rule_.messageElements_.end();
        ++m_it) {
    if ((*m_it)->elementType_ != AUDIO_MEDIAPLAYER) {
      nonMultimediaCnt++;
    } else {
      /// Write the audio file into 'multiMediaFileDir'
      std::string filePath = multiMediaFileDir + "/" + (*m_it)->url_;
      AudioHandledInfo audioHandledInfo;
      std::vector<std::string> audioFileNameSegs;
      SplitString((*m_it)->url_, '.', &audioFileNameSegs);
      audioHandledInfo.origAudioFileName = audioFileNameSegs[0];  // origAudioFileName
      std::string fileExtention = audioFileNameSegs[1];           // origAudioFormat
      StringToLowerCase(fileExtention);  // toLowerCase
      if ("ogg" == fileExtention) {
        audioHandledInfo.origAudioFormat = OGG;
      } else if ("amr" == fileExtention) {
        audioHandledInfo.origAudioFormat = AMR;
      } else if ("wav" == fileExtention) {
        audioHandledInfo.origAudioFormat = WAV;
      } else if ("mp3" == fileExtention) {
        audioHandledInfo.origAudioFormat = MP3;
      }
      audioHandledInfo.origAudioFilePath = filePath;
      file::File::WriteStringToFile(
            (*m_it)->elementContent_->GetElementContent(), filePath);
      elementAudioHandledMap.insert(std::pair<MessageElement*, AudioHandledInfo>(
              *m_it, audioHandledInfo));
    }
  }

  /*
   * If have Audio, then AudioSynthesize
   */
  AudioHandledInfo synthesizeAudioInfo;
  if (!elementAudioHandledMap.empty()) {
    news::Timer synthesizeAudioTime;
    synthesizeAudioTime.Begin();
    if (!AudioSynthesize(multiMediaFileDir, elementAudioHandledMap,
            synthesizeAudioInfo)) {
      LOG(ERROR) << "AudioSynthesize error when MagicMessage2Video";
      file::File::DeleteRecursively(multiMediaFileDir);
      return false;
    }
    synthesizeAudioTime.End();
    VLOG(4) << "-------- AudioSynthesize consume time: "
      << synthesizeAudioTime.GetMs() << "ms";
  }
  
  VLOG(4) << nonMultimediaCnt << " Non-MultiMedia elements, Start "
    << nonMultimediaCnt << " threads to handle these Non-MultiMedia elements";
  /*
   * Start multi-threads to handle all the non-multimedia elements
   */
  std::map<int, ElementHandledInfo<unsigned char> > elementsCommonBuffer;
  Mutex elementsCommonBufferMutex;
  RwMutex elementsAllDoneRwMutex;
  base::CondVar elementsHandledCondVar;
  shared_ptr<HandleElementThread> handleThreads[nonMultimediaCnt];
  int64_t startTime = news::Timer::CurrentTimeInMs();
  int64_t endTime = startTime + magic_message_control_rule_.duration_;
  bool allElementsDone = false;  // must be false
  
  VLOG(4) << "common-startTime: " << startTime;
  int id = 0;
  int elementsDatumTime = 0;
  for (std::vector<MessageElement*>::iterator m_it =
        magic_message_control_rule_.messageElements_.begin();
        m_it != magic_message_control_rule_.messageElements_.end();
        ++m_it) {
    /// Only handle the Non-MultiMedia element
    if ((*m_it)->elementType_ != AUDIO_MEDIAPLAYER) {
      VLOG(4) << "Create thread current time: " << news::Timer::CurrentTimeInMs();
      handleThreads[id] = shared_ptr<HandleElementThread>(
            new HandleElementThread(
              nonMultimediaCnt,  // the total thread num
              id,                // the element sequence id
              startTime,         // the all elements common start-time
              endTime,           // the all elements common end-time
              &elementsDatumTime,
              *m_it,             // this element's pointer
              &elementsCommonBuffer,
              &elementsCommonBufferMutex,
              &elementsAllDoneRwMutex,
              &elementsHandledCondVar,
              &allElementsDone));  // the all elements common data buffer rw-mutex
      handleThreads[id]->Start();
      id++;
    }
  }

  int subFrameCnt = 0;
  int frameCnt = 0;
  while (true) {
    std::map<int, ElementHandledInfo<unsigned char> > elementsBufferOut;
    /// According the elementsDatumTime, decide whether allElementsDone
    //elementsAllDoneRwMutex.WriterLock();    // WriteLock
    elementsCommonBufferMutex.Lock();    // Lock
    if (elementsDatumTime > magic_message_control_rule_.duration_) {
      allElementsDone = true;
    }
    //elementsAllDoneRwMutex.WriterUnlock();  // WriteUnlock
    elementsCommonBufferMutex.Unlock();  // Unlock
    
    if (allElementsDone) {  // All elements have done
      VLOG(4) << "Have handled all elements in multi-thread mode done, "
        << "handled " << frameCnt << " frames";
      break;
    }

    elementsCommonBufferMutex.Lock();    // Lock
    while (!allElementsDone 
          && elementsCommonBuffer.size() < nonMultimediaCnt) {
      elementsHandledCondVar.Wait(&elementsCommonBufferMutex);
    }
    subFrameCnt += elementsCommonBuffer.size();
    frameCnt++;
    elementsBufferOut = elementsCommonBuffer;
    elementsCommonBuffer.clear();        // Clear the elements common buffer
    elementsDatumTime += SAMPLE_HANDLE_INTERVAL;
    elementsCommonBufferMutex.Unlock();  // Unlock

    /// Merge all sub-frame into one frame
    Magick::Image frameBackGround(
          Magick::Geometry(magic_message_control_rule_.screenWidth_,
            magic_message_control_rule_.screenHeight_),
          Magick::Color("transparent"));
    frameBackGround.backgroundColor(Magick::Color("transparent"));
    for (int i = 0; i < nonMultimediaCnt; i++) {
      if (elementsBufferOut.find(i) == elementsBufferOut.end()) {
        continue;
      }
      if (elementsBufferOut[i].isVisible_) {  // Visible, composite
        //int imgW = elementsBufferOut[i].handledImg_.columns();
        //int imgH = elementsBufferOut[i].handledImg_.rows();
        int xOff = elementsBufferOut[i].xOff_;
        int yOff = elementsBufferOut[i].yOff_;
        frameBackGround.composite(
              elementsBufferOut[i].handledImg_,
              //Magick::Geometry(imgW, imgH),
              xOff, yOff,
              Magick::OverCompositeOp);
      }
    }
    frameBackGround.write(std::string(multiMediaFileDir + "/" + IntToString(frameCnt) + ".jpg").c_str());  // ".jpg"
    elementsBufferOut.clear();
  }

  VLOG(4) << "[TEST] The total frameCnt: " << frameCnt
    << ", total subFrameCnt: " << subFrameCnt;
  /// Start merge all handled frames into a video
  std::string magic_video_file = multiMediaFileDir + "/" + FLAGS_default_video_name;

  std::string makeMagicVideo = "ffmpeg -r ";
  makeMagicVideo += DoubleToString((double)1000 / SAMPLE_HANDLE_INTERVAL);
  makeMagicVideo += " -f image2 -i ";
  makeMagicVideo += multiMediaFileDir;
  makeMagicVideo += "/%d.jpg ";
  if (synthesizeAudioInfo.isHandled_) {
    /// If have handled audio file, merge the audio into the video file
    makeMagicVideo += " -i ";
    makeMagicVideo += synthesizeAudioInfo.handledAudioFilePath;
  }
  makeMagicVideo += " -qscale 5 -strict -2 -vcodec h264 ";
  // makeMagicVideo += " -strict -2 ";
  makeMagicVideo += magic_video_file;
  VLOG(4) << "makeMagicVideoCommand: " << makeMagicVideo;
  if (system(makeMagicVideo.c_str()) > 0) {
    VLOG(4) << "MakeMagicVideo successfully";
  }

  if (!file::File::ReadFileToString(magic_video_file,
        &magicMessageVideoContent)) {
    LOG(ERROR) << "Read the MagicMessageVideo content error "
      << "when MagicMessage2Video with magicVideoFile = "
      << magic_video_file;
    // file::File::DeleteRecursively(multiMediaFileDir);
    return false;
  }
  /// Delete the tmp Video-Made dir
  //file::File::DeleteRecursively(multiMediaFileDir);
  return true;
}

bool MagicMessageControl::ExtractElement(const Json::Value& elements) {
  if (elements.empty()) {
    LOG(ERROR) << "Empty elements when ExtractElement";
    return false;
  }
  VLOG(4) << "Have " << elements.size() << " element in this MagicMessage";
  for (int i = 0; i < elements.size(); i++) {
    /// ***** for one element *****
    MessageElement* messageElement = new MessageElement(
          magic_message_control_rule_.screenWidth_,
          magic_message_control_rule_.screenHeight_);
    if (!elements[i].isMember("actions")) {
      LOG(ERROR) << "Null actions in this MagicMessage when ExtractElement"
        << " with elements index = " << i;
      delete messageElement;
      messageElement = NULL;
      return false;
    }
    // for this elements' all actions
    if (!ExtractAction(elements[i]["actions"],
            messageElement->actions_)) {
      LOG(ERROR) << "ExtractAction error when ExtractElement"
        << " with elements index = " << i;
      delete messageElement;
      messageElement = NULL;
      return false;
    }

    if (elements[i].isMember("id")) {
      messageElement->id_ = elements[i]["id"].asString();
    }

    /// Assign the elementType
    if (elements[i].isMember("type")) {
      std::string typeName = elements[i]["type"].asString();
      if ("audio_mediaplayer" == typeName) {  // audio_mediaplayer
        messageElement->elementType_ = AUDIO_MEDIAPLAYER;
      } else if ("gif" == typeName) {         // GIF or ZIP_GIF
        std::string gifFileExtentionName = GetFileExtentionName(
              elements[i]["url"].asString());
        if ("zip" == gifFileExtentionName) {
          messageElement->elementType_ = ZIP_GIF;
        } else if ("gif" == gifFileExtentionName) {
          messageElement->elementType_ = GIF;
        }
      } else if ("picture" == typeName) {  // PIC
        messageElement->elementType_ = IMG;
      } else if ("text" == typeName) {     // TEXT
        messageElement->elementType_ = TEXT;
      } else if ("video" == typeName) {    // VIDEO
        messageElement->elementType_ = VIDEO;
      } else if ("picture_bg" == typeName) {  // BackGround picture
        messageElement->elementType_ = BACKGROUND_IMG; 
      }
    }
    if (elements[i].isMember("url")) {
      messageElement->url_ = elements[i]["url"].asString();
    }
    if (elements[i].isMember("isRerun")) {
      messageElement->isReturn_ = elements[i]["isRerun"].asBool();
    }
    if (elements[i].isMember("x")) {
      messageElement->x_ = elements[i]["x"].asInt();
    }
    if (elements[i].isMember("y")) {
      messageElement->y_ = elements[i]["y"].asInt();
    }
    if (elements[i].isMember("w")) {
      messageElement->w_ = elements[i]["w"].asInt();
    }
    if (elements[i].isMember("h")) {
      messageElement->h_ = elements[i]["h"].asInt();
    }
    if (elements[i].isMember("scale")) {
      messageElement->scale_ = elements[i]["scale"].asDouble();
    }
    if (elements[i].isMember("degree")) {
      messageElement->degree_ = elements[i]["degree"].asDouble();
    }
    if (elements[i].isMember("visible")) {
      messageElement->visible_ = elements[i]["visible"].asBool();
    }
    if (elements[i].isMember("volume")) {  // for VIDEO and AUDIO
      messageElement->volume_ = elements[i]["volume"].asInt();
    }
    /// for TEXT as below
    if (elements[i].isMember("text")) {
      messageElement->text_ = elements[i]["text"].asString();
      messageElement->url_ = elements[i]["text"].asString();  // for TEXT: url_ == text_
    }
    if (elements[i].isMember("size")) {
      messageElement->size_ = elements[i]["size"].asInt();
    }
    if (elements[i].isMember("textScreenW")) {
      messageElement->textScreenW_ = elements[i]["textScreenW"].asInt();
    }
    if (elements[i].isMember("textScreenH")) {
      messageElement->textScreenH_ = elements[i]["textScreenH"].asInt();
    }
    if (elements[i].isMember("color")) {
      messageElement->color_ = elements[i]["color"].asInt();
    }
    if (elements[i].isMember("tf")) {
      messageElement->tf_ = elements[i]["tf"].asString();
    }
    if (elements[i].isMember("direction")) {
      messageElement->direction_ = elements[i]["direction"].asInt();
    }
    if (elements[i].isMember("duration")) {
      messageElement->duration_ = elements[i]["duration"].asInt();
    }
    magic_message_control_rule_.messageElements_.push_back(
          messageElement);  // Here is a pointer
  }
  return true;
}

bool MagicMessageControl::ExtractAction(
      const Json::Value& actions_json,
      std::vector<MsgAction*>& actions) {
#if 0
  /// if the actions_json.empty(), only disply it
  if (actions_json.empty()) {
    LOG(ERROR) << "Empty actions Node when ExtractAction";
    return false;
  }
#endif
  Json::Value::Members members = actions_json.getMemberNames();
  for (int i = 0; i < members.size(); i++) {
    MsgAction* msg_action;
    std::string member = members[i];
    if ("rotate" == member) {
      for (int i = 0; i < actions_json[member].size(); i++) {
        msg_action = new MsgActionRotate;
        msg_action->SetActionType();
        if (!msg_action->ParseActionStatusData(actions_json[member][i])) {
          LOG(ERROR) << "ParseActionStatusData error"
            << " with actionName = " << member
            << ", jsonIndex = " << i;
          return false;
        }
        actions.push_back(msg_action);
      }
    } else if ("scale" == member) {
      for (int i = 0; i < actions_json[member].size(); i++) {
        msg_action = new MsgActionScale;
        msg_action->SetActionType();
        if (!msg_action->ParseActionStatusData(actions_json[member][i])) {
          LOG(ERROR) << "ParseActionStatusData error"
            << " with actionName = " << member
            << ", jsonIndex = " << i;
          return false;
        }
        actions.push_back(msg_action);
      }
    } else if ("translate" == member) {
      for (int i = 0; i < actions_json[member].size(); i++) {
        msg_action = new MsgActionTranslate;
        msg_action->SetActionType();
        if (!msg_action->ParseActionStatusData(actions_json[member][i])) {
          LOG(ERROR) << "ParseActionStatusData error"
            << " with actionName = " << member
            << ", jsonIndex = " << i;
          return false;
        }
        actions.push_back(msg_action);
      }
    } else if ("visibility" == member) {
      for (int i = 0; i < actions_json[member].size(); i++) {
        msg_action = new MsgActionVisibility;
        msg_action->SetActionType();
        if (!msg_action->ParseActionStatusData(actions_json[member][i])) {
          LOG(ERROR) << "ParseActionStatusData error"
            << " with actionName = " << member
            << ", jsonIndex = " << i;
          return false;
        }
        actions.push_back(msg_action);
      }
    } else if ("play" == member) {
      for (int i = 0; i < actions_json[member].size(); i++) {
        msg_action = new MsgActionPlay;
        msg_action->SetActionType();
        if (!msg_action->ParseActionStatusData(actions_json[member][i])) {
          LOG(ERROR) << "ParseActionStatusData error"
            << " with actionName = " << member
            << ", jsonIndex = " << i;
          return false;
        }
        actions.push_back(msg_action);
      }
    } else if ("textAnimate" == member) {
      for (int i = 0; i < actions_json[member].size(); i++) {
        msg_action = new MsgActionTextAnimate;
        msg_action->SetActionType();
        if (!msg_action->ParseActionStatusData(actions_json[member][i])) {
          LOG(ERROR) << "ParseActionStatusData error"
            << " with actionName = " << member
            << ", jsonIndex = " << i;
          return false;
        }
        actions.push_back(msg_action);
      }
    } else {
      LOG(ERROR) << "Unknown actionName when ExtractAction";
      return false;
    }
  }

  /// Sort by all MsgAction's startTime ASC Order
  std::sort(actions.begin(), actions.end(),
        CompareMsgActionByStartTimeASC);
  /// TEST
  for (int i = 0; i < actions.size(); i++) {
    VLOG(4) << i << "th actions: "
      << "actionType_ = " << actions[i]->actionType_
      << ", startTime_ = " << actions[i]->startTime_
      << ", duration_ = " << actions[i]->duration_;
  }
  return true;
}

std::string GetFileExtentionName(const std::string& fileName) {
  if (fileName.empty()) {
    return std::string("");
  }
  if (fileName.rfind(".") == std::string::npos) {
    return std::string("");
  }
  std::string fileExtentionName = fileName.substr(
        fileName.rfind(".") + 1);
  VLOG(4) << "fileExtentionName: " << fileExtentionName;
  if (fileExtentionName.length() > 4) {
    return std::string("");
  }
  return fileExtentionName;
}

bool CompareMsgActionByStartTimeASC(const MsgAction* msgPtr1,
      const MsgAction* msgPtr2) {
  return msgPtr1->startTime_ < msgPtr2->startTime_;
}

template<class T>
int MaxFinalTimeStamp(
      const std::map<int, ElementHandledInfo<T> >& idElementHandledMap) {
  int finalTime = 0;
  for (class std::map<int, ElementHandledInfo<T> >::const_iterator
        it = idElementHandledMap.begin();
        it != idElementHandledMap.end(); ++it) {
    if (it->finalTimeStamp_ > finalTime) {
      finalTime = it->finalTimeStamp_;
    }
  }
  return finalTime;
}

void StringToLowerCase(std::string& data) {
  std::transform(data.begin(), data.end(), data.begin(), ::tolower);
}
}  // namespace multimedia
}  // namespace juyou
