// Copyright (c) 2013 Juyou Inc. All rights reserved.
// Author: wangjuntao@juyou.com (Juntao Wang)

#include <string>
#include <Magick++.h>
#include "base/logging.h"
#include "base/flags.h"
#include "news_search/base/json_config/config.h"
#include "util/global_init/global_init.h"
#include "open_magic_message_server.h"

using news_search::Config;  // NOLINT

DEFINE_string(config_file, "",
      "The configuration file");

int main(int argc, char* argv[]) {
  util::GlobalInit global_init(&argc, &argv);
  // Magick::InitializeMagick("OpenMagicMessageServer--Magick");
#if 0
  /// Read configuration file
  if (!Config::Init(FLAGS_config_file)) {
    LOG(FATAL) << "Failed to read config -- "
      << FLAGS_config_file;
    exit(0);
  }
#endif

  /// Start OpenMagicMessageServer
  ::juyou::multimedia::OpenMagicMessageServer open_magic_message_server;
  try {
    open_magic_message_server.Serve();
    LOG(INFO) << "Have started OpenMagicMessageServer in main processor successfully";
  } catch(const std::exception& err) {
    LOG(ERROR) << "Failed to start OpenMagicMessageServer in main processor: "
      << err.what();
  } catch(...) {
    LOG(ERROR) << "Failed to start OpenMagicMessageServer in main processor "
      << "unexplained exception";
  }
  return 0;
}
