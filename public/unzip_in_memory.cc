// Copyright (c) 2013 Juyou Inc. All rights reserved.
// Author: wangjuntao@juyou.com (Juntao Wang)

/// http://stackoverflow.com/questions/10781693/how-to-unzip-a-zip-file-already-loaded-in-memory-in-c
int FileContent() {
  struct archive *a = archive_read_new();
  archive_read_support_compression_gzip(a);
  archive_read_support_format_tar(a);
  r = archive_read_open_memory(a, buff, sizeof(buff));
}


/// http://stackoverflow.com/questions/19277721/unzipping-an-encrypted-file-in-bytes/19558614#19558614
zlib_filefunc_def FileFunctions32 = {0};
ourmemory_t *FileMemory = {0};

// Fill out ourmemory_t instance with buffer pointer (base) and length (size)
//
fill_memory_filefunc(&FileFunctions32, FileMemory);
UnzipFileHandle = unzOpen2("__notused__", &FileFunctions32);
