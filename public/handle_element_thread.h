// Copyright (c) 2013 Juyou Inc. All rights reserved.
// Author: wangjuntao@juyou.com (Juntao Wang)

#ifndef JUYOU_MULTI_MEDIA_PUBLIC_HANDLE_ELEMENT_THREAD_H
#define JUYOU_MULTI_MEDIA_PUBLIC_HANDLE_ELEMENT_THREAD_H_

#include <string>
#include <vector>
#include <map>
#include "base/thread.h"
#include "base/logging.h"
#include "base/flags.h"
#include "base/time.h"
#include "base/basictypes.h"
#include "base/mutex.h"
#include "juyou/multi_media/public/message_element.h"

using base::RwMutex;  // NOLINT
using base::Mutex;    // NOLINT
using base::RwMutex;  // NOLINT
using base::CondVar;  // NOLINT

namespace juyou {
namespace multimedia {

class HandleElementThread : public base::Thread {
 public:
  HandleElementThread(int totalThreadNum, int elementSeq,
        int64_t startTime, int64_t endTime,
        int* elementDatumTime,
        MessageElement* messageElement,
        std::map<int, ElementHandledInfo<unsigned char> >* elementsCommonBuffer,
        Mutex* elementsBufferMutext,
        RwMutex* isAllDoneRwMutex,
        CondVar* elementsHandledCondVar,
        bool* isAllDone);
  
  virtual ~HandleElementThread();

  virtual void Run();
 private:
  bool HandleElementByDatumTime(const int& datumTime,
        ElementHandledInfo<unsigned char>& elementHandledInfo);

 private:
  int totalThreadNum_;
  int elementSeq_;
  int64_t startTime_;
  int64_t endTime_;
  int* elementsDatumTime_;
  MessageElement* messageElement_;  // Pointer to the MessageElement which will be handled
  std::map<int, ElementHandledInfo<unsigned char> >* elementsCommonBuffer_;
  Mutex* elementsBufferMutex_;
  RwMutex* isAllDoneRwMutex_;
  CondVar* elementsHandledCondVar_;
  bool* isAllDone_;
 private:
  int threadDatumTime_;
};
}  // namespace juyou
}  // namespace multimedia
#endif  // JUYOU_MULTI_MEDIA_PUBLIC_HANDLE_ELEMENT_THREAD_H_
