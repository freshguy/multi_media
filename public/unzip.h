// Copyright (c) 2013 Juyou Inc. All rights reserved.
// Author: wangjuntao@juyou.com (Juntao Wang)
// modify from 'miniunz.c'

#ifndef JUYOU_MULTI_MEDIA_PUBLIC_UNZIP_H_
#define JUYOU_MULTI_MEDIA_PUBLIC_UNZIP_H_

#if (!defined(_WIN32)) && (!defined(WIN32)) && (!defined(__APPLE__))
  #ifndef __USE_FILE_OFFSET64
    #define __USE_FILE_OFFSET64
  #endif

  #ifndef __USE_LARGEFILE64
    #define __USE_LARGEFILE64
  #endif

  #ifndef _LARGEFILE64_SOURCE
    #define _LARGEFILE64_SOURCE
  #endif

  #ifndef _FILE_OFFSET_BIT
    #define _FILE_OFFSET_BIT 64
  #endif
#endif

#ifdef __APPLE__
// In darwin and perhaps other BSD variants off_t is a 64 bit value, hence no need for specific 64 bit functions
#define FOPEN_FUNC(filename, mode) fopen(filename, mode)
#define FTELLO_FUNC(stream) ftello(stream)
#define FSEEKO_FUNC(stream, offset, origin) fseeko(stream, offset, origin)
#else
#define FOPEN_FUNC(filename, mode) fopen64(filename, mode)
#define FTELLO_FUNC(stream) ftello64(stream)
#define FSEEKO_FUNC(stream, offset, origin) fseeko64(stream, offset, origin)
#endif


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <utime.h>

#include <sys/types.h> 
#include <sys/stat.h>
#include "juyou/base/zlib/contrib/minizip/unzip.h"  // from zlib

#define CASESENSITIVITY (0)
#define WRITEBUFFERSIZE (8192)
#define MAXFILENAME (256)

/* change_file_date : change the date/time of a file
filename : the filename of the file where date/time must be modified
dosdate : the new date at the MSDos format (4 bytes)
tmu_date : the SAME new date at the tm_unz format */

void change_file_date(const char* filename,
      uLong dosdate, tm_unz tmu_date);


/* mymkdir and change_file_date are not 100 % portable
   As I don't know well Unix, I wait feedback for the unix portion */

int mymkdir(const char* dirname);

int makedir (char* newdir);

void Display64BitsSize(ZPOS64_T n, int size_char);

int do_list(unzFile uf);

int do_extract_currentfile(unzFile uf,
      const int* popt_extract_without_path,
      int* popt_overwrite,
      const char* password);

int do_extract(unzFile uf,
      int opt_extract_without_path,
      int opt_overwrite,
      const char* password);

int do_extract_onefile(unzFile uf,
      const char* filename,
      int opt_extract_without_path,
      int opt_overwrite,
      const char* password);
#endif  // JUYOU_MULTI_MEDIA_PUBLIC_UNZIP_H_
