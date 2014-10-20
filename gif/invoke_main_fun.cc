// Copyright (c) 2013 Juyou Inc. All rights reserved.
// Author: wangjuntao@juyou.com (Juntao Wang)

#include <string>
#include <vector>
#include "base/logging.h"
#include "base/flags.h"

int Tmain(int argc, char** argv) {
  LOG(INFO) << "Main function";
  LOG(INFO) << "argc: " << argc;
  for (int i = 0; i < argc; i++) {
    LOG(INFO) << i << "th argv: " << argv[i];
  }
  return 0;
}

int main(int argc, char** argv) {
  int ARGC = 3;
  char* ARGV[] = {(char*)"invoke_test",
  (char*)"good good study",
  (char*)"day day up!"};
  int mainRet = Tmain(ARGC, ARGV);
  LOG(INFO) << "Tmain ret: " << mainRet;
  return 0;
}
