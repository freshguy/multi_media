// Copyright (c) 2013 Juyou Inc. All rights reserved.
// Author: wangjuntao@juyou.com (Juntao Wang)

#include "juyou/multi_media/public/handle_element_thread.h"

namespace juyou {
namespace multimedia {

HandleElementThread::HandleElementThread(
      int totalThreadNum, int elementSeq,
      int64_t startTime, int64_t endTime,
      int* elementDatumTime,
      MessageElement* messageElement,
      std::map<int, ElementHandledInfo<unsigned char> >* elementsCommonBuffer,
      Mutex* elementsBufferMutex,
      RwMutex* isAllDoneRwMutex,
      CondVar* elementsHandledCondVar,
      bool* isAllDone) {
  totalThreadNum_ = totalThreadNum;
  elementSeq_ = elementSeq;
  startTime_ = startTime;
  endTime_ = endTime;
  elementsDatumTime_ = elementDatumTime;
  messageElement_ = messageElement;
  elementsCommonBuffer_ = elementsCommonBuffer;
  elementsBufferMutex_ = elementsBufferMutex;
  isAllDoneRwMutex_ = isAllDoneRwMutex;
  elementsHandledCondVar_ = elementsHandledCondVar;
  isAllDone_ = isAllDone;
  threadDatumTime_ = -1;   // Must initialize with value(< 0)
}

HandleElementThread::~HandleElementThread() {
  VLOG(4) << "----------- Thread " << elementSeq_
    << " destroyed ----------";
}

void HandleElementThread::Run() {
  while (true) {
    /// Check whether all elements allDone
    //isAllDoneRwMutex_->ReaderLock();  // Only read
    elementsBufferMutex_->Lock();
    int handleDatumTime = 0;
    if (*isAllDone_) {
      //isAllDoneRwMutex_->ReaderUnlock();
      elementsBufferMutex_->Unlock();
      break;
    }
    handleDatumTime = *elementsDatumTime_;
    if (handleDatumTime <= threadDatumTime_) {
      elementsBufferMutex_->Unlock();
      continue;
    }
    //isAllDoneRwMutex_->ReaderUnlock();
    elementsBufferMutex_->Unlock();

    bool isSignal = false;  // The flag mark whether this thread delivery the Signal
    //threadDatumTime_ = handleDatumTime;
    //ElementHandledInfo<unsigned char> elementHandledInfo;
    //HandleElementByDatumTime(handleDatumTime, elementHandledInfo);
    
    elementsBufferMutex_->Lock();
#if 0
    if (elementsCommonBuffer_->size() == totalThreadNum_) {  // elementsCommonBuffer_
      elementsHandledCondVar_->Signal();
      VLOG(5) << "Thread - " << elementSeq_ << " delivery CondVar-Signal";
      isSignal = true;
    } else if (elementsCommonBuffer_->find(elementSeq_)
          == elementsCommonBuffer_->end()) {  // (elementsCommonBuffer_->size() < totalThreadNum_){
      /// Here we handle this frame
      threadDatumTime_ = handleDatumTime;
      ElementHandledInfo<unsigned char> elementHandledInfo;
      HandleElementByDatumTime(handleDatumTime, elementHandledInfo);
      elementsCommonBuffer_->insert(
            std::pair<int, ElementHandledInfo<unsigned char> >(
              elementSeq_, elementHandledInfo));
      VLOG(5) << "Thread - " << elementSeq_ << " have made a frame";
    }
#endif
    if (elementsCommonBuffer_->find(elementSeq_)
          == elementsCommonBuffer_->end()) {
      threadDatumTime_ = handleDatumTime;
      ElementHandledInfo<unsigned char> elementHandledInfo;
      HandleElementByDatumTime(handleDatumTime, elementHandledInfo);
      elementsCommonBuffer_->insert(
            std::pair<int, ElementHandledInfo<unsigned char> >(
              elementSeq_, elementHandledInfo));
      VLOG(5) << "Thread - " << elementSeq_ << " have made a frame";
      if (elementsCommonBuffer_->size() == totalThreadNum_) {  // elementsCommonBuffer_
        elementsHandledCondVar_->Signal();
        VLOG(5) << "Thread - " << elementSeq_ << " delivery CondVar-Signal";
        isSignal = true;
      }
    }

#if 0
    elementsCommonBuffer_->insert(
          std::pair<int, ElementHandledInfo<unsigned char> >(
            elementSeq_, elementHandledInfo));
    //VLOG(4) << "Thread - " << elementSeq_
      //<< " have made frame for datumTime: " << threadDatumTime_;
    if (elementsCommonBuffer_->size() == totalThreadNum_) {  // elementsCommonBuffer_
      elementsHandledCondVar_->Signal();
      VLOG(5) << "Thread - " << elementSeq_ << " delivery CondVar-Signal";
      isSignal = true;
    }
#endif
    elementsBufferMutex_->Unlock();
  }
}
  
bool HandleElementThread::HandleElementByDatumTime(
      const int& datumTime,
      ElementHandledInfo<unsigned char>& elementHandledInfo) {
  elementHandledInfo.handledStatus_ = FRAME_DONE;
  elementHandledInfo.seqId_ = elementSeq_;
  if (!messageElement_->HandleElementByActions(datumTime,
        elementHandledInfo)) {
    return false;
  }
  return true;
}
}  // namespace multimedia
}  // namespace juyou
