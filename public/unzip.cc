// Copyright (c) 2013 Juyou Inc. All rights reserved.
// Author: wangjuntao@juyou.com (Juntao Wang)

#include "base/logging.h"
#include "base/flags.h"
#include "juyou/multi_media/public/unzip.h"

void change_file_date(const char* filename,
      uLong dosdate, tm_unz tmu_date) {
  struct utimbuf ut;
  struct tm newdate;
  newdate.tm_sec = tmu_date.tm_sec;
  newdate.tm_min=tmu_date.tm_min;
  newdate.tm_hour=tmu_date.tm_hour;
  newdate.tm_mday=tmu_date.tm_mday;
  newdate.tm_mon=tmu_date.tm_mon;
  if (tmu_date.tm_year > 1900)
  newdate.tm_year=tmu_date.tm_year - 1900;
  else
  newdate.tm_year=tmu_date.tm_year ;
  newdate.tm_isdst=-1;

  ut.actime=ut.modtime=mktime(&newdate);
  utime(filename,&ut);
}


int mymkdir(const char* dirname) {
  int ret=0;
  ret = mkdir (dirname,0775);
  return ret;
}

int makedir (char* newdir) {
  char *buffer ;
  char *p;
  int  len = (int)strlen(newdir);

  if (len <= 0)
  return 0;

  buffer = (char*)malloc(len+1);
  if (buffer==NULL)
  {
    LOG(ERROR) << "Error allocating memory";
    return UNZ_INTERNALERROR;
  }
  strcpy(buffer,newdir);

  if (buffer[len-1] == '/') {
    buffer[len-1] = '\0';
  }
  if (mymkdir(buffer) == 0)
  {
    free(buffer);
    return 1;
  }

  p = buffer+1;
  while (1)
  {
    char hold;

    while(*p && *p != '\\' && *p != '/')
    p++;
    hold = *p;
    *p = 0;
    if ((mymkdir(buffer) == -1) && (errno == ENOENT))
    {
      LOG(ERROR) << "Couldn't create directory " << buffer;
      free(buffer);
      return 0;
    }
    if (hold == 0)
    break;
    *p++ = hold;
  }
  free(buffer);
  return 1;
}

void Display64BitsSize(ZPOS64_T n, int size_char) {
  /* to avoid compatibility problem , we do here the conversion */
  char number[21];
  int offset=19;
  int pos_string = 19;
  number[20]=0;
  for (;;) {
    number[offset]=(char)((n%10)+'0');
    if (number[offset] != '0')
    pos_string=offset;
    n/=10;
    if (offset==0)
    break;
    offset--;
  }
  {
    int size_display_string = 19-pos_string;
    while (size_char > size_display_string)
    {
      size_char--;
      printf(" ");
    }
  }

  printf("%s",&number[pos_string]);
}

int do_list(unzFile uf) {
  uLong i;
  unz_global_info64 gi;
  int err;

  err = unzGetGlobalInfo64(uf,&gi);
  if (err!=UNZ_OK)
  printf("error %d with zipfile in unzGetGlobalInfo \n",err);
  printf("  Length  Method     Size Ratio   Date    Time   CRC-32     Name\n");
  printf("  ------  ------     ---- -----   ----    ----   ------     ----\n");
  for (i=0;i<gi.number_entry;i++)
  {
    char filename_inzip[256];
    unz_file_info64 file_info;
    uLong ratio=0;
    const char *string_method;
    char charCrypt=' ';
    err = unzGetCurrentFileInfo64(uf,&file_info,filename_inzip,sizeof(filename_inzip),NULL,0,NULL,0);
    if (err!=UNZ_OK)
    {
      printf("error %d with zipfile in unzGetCurrentFileInfo\n",err);
      break;
    }
    if (file_info.uncompressed_size>0)
    ratio = (uLong)((file_info.compressed_size*100)/file_info.uncompressed_size);

    /* display a '*' if the file is crypted */
    if ((file_info.flag & 1) != 0)
    charCrypt='*';

    if (file_info.compression_method==0)
    string_method="Stored";
    else
    if (file_info.compression_method==Z_DEFLATED)
    {
      uInt iLevel=(uInt)((file_info.flag & 0x6)/2);
      if (iLevel==0)
      string_method="Defl:N";
      else if (iLevel==1)
      string_method="Defl:X";
      else if ((iLevel==2) || (iLevel==3))
      string_method="Defl:F"; /* 2:fast , 3 : extra fast*/
    }
    else
    if (file_info.compression_method==Z_BZIP2ED)
    {
      string_method="BZip2 ";
    }
    else
    string_method="Unkn. ";

    Display64BitsSize(file_info.uncompressed_size,7);
    printf("  %6s%c",string_method,charCrypt);
    Display64BitsSize(file_info.compressed_size,7);
    printf(" %3lu%%  %2.2lu-%2.2lu-%2.2lu  %2.2lu:%2.2lu  %8.8lx   %s\n",
          ratio,
          (uLong)file_info.tmu_date.tm_mon + 1,
          (uLong)file_info.tmu_date.tm_mday,
          (uLong)file_info.tmu_date.tm_year % 100,
          (uLong)file_info.tmu_date.tm_hour,(uLong)file_info.tmu_date.tm_min,
          (uLong)file_info.crc,filename_inzip);
    if ((i+1)<gi.number_entry)
    {
      err = unzGoToNextFile(uf);
      if (err!=UNZ_OK)
      {
        printf("error %d with zipfile in unzGoToNextFile\n",err);
        break;
      }
    }
  }
  return 0;
}


int do_extract_currentfile(unzFile uf,
      const int* popt_extract_without_path,
      int* popt_overwrite,
      const char* password) {
  char filename_inzip[256];
  char* filename_withoutpath;
  char* p;
  int err=UNZ_OK;
  FILE *fout=NULL;
  void* buf;
  uInt size_buf;

  unz_file_info64 file_info;
  err = unzGetCurrentFileInfo64(uf,
        &file_info,filename_inzip,
        sizeof(filename_inzip), NULL, 0, NULL, 0);

  if (err!=UNZ_OK) {
    LOG(ERROR) << "error " << err << " with zipfile in unzGetCurrentFileInfo";
    return err;
  }

  size_buf = WRITEBUFFERSIZE;
  buf = (void*)malloc(size_buf);
  if (buf == NULL) {
    LOG(ERROR) << "Error allocating memory";
    return UNZ_INTERNALERROR;
  }

  p = filename_withoutpath = filename_inzip;
  while ((*p) != '\0')
  {
    if (((*p)=='/') || ((*p)=='\\'))
    filename_withoutpath = p+1;
    p++;
  }

  if ((*filename_withoutpath)=='\0')
  {
    if ((*popt_extract_without_path)==0)
    {
      VLOG(5) << "creating directory: " << filename_inzip;
      mymkdir(filename_inzip);
    }
  }
  else
  {
    const char* write_filename;
    int skip=0;

    if ((*popt_extract_without_path)==0)
    write_filename = filename_inzip;
    else
    write_filename = filename_withoutpath;

    err = unzOpenCurrentFilePassword(uf,password);
    if (err!=UNZ_OK) {
      LOG(ERROR) << "error " << err << " with zipfile in unzOpenCurrentFilePassword";
    }

    if (((*popt_overwrite)==0) && (err==UNZ_OK))
    {
      char rep=0;
      FILE* ftestexist;
      ftestexist = FOPEN_FUNC(write_filename,"rb");
      if (ftestexist!=NULL)
      {
        fclose(ftestexist);
#if 0
        do
        {
          char answer[128];
          int ret;

          printf("The file %s exists. Overwrite ? [y]es, [n]o, [A]ll: ",write_filename);
          ret = scanf("%1s",answer);
          if (ret != 1)
          {
            exit(EXIT_FAILURE);
          }
          rep = answer[0] ;
          if ((rep>='a') && (rep<='z'))
          rep -= 0x20;
        }
        while ((rep!='Y') && (rep!='N') && (rep!='A'));
#endif
        /// Always all overwrite
        rep == 'A';
      }

      if (rep == 'N')
      skip = 1;

      if (rep == 'A')
      *popt_overwrite=1;
    }

    if ((skip==0) && (err==UNZ_OK))
    {
      fout=FOPEN_FUNC(write_filename,"wb");
      /* some zipfile don't contain directory alone before file */
      if ((fout==NULL) && ((*popt_extract_without_path)==0) &&
            (filename_withoutpath!=(char*)filename_inzip))
      {
        char c=*(filename_withoutpath-1);
        *(filename_withoutpath-1)='\0';
        makedir((char*)write_filename);
        *(filename_withoutpath-1)=c;
        fout=FOPEN_FUNC(write_filename, "wb");
      }

      if (fout==NULL)
      {
        LOG(ERROR) << "error opening " << write_filename;
      }
    }

    if (fout!=NULL)
    {
      // printf(" extracting: %s\n",write_filename);
      do
      {
        err = unzReadCurrentFile(uf,buf,size_buf);
        if (err<0)
        {
          LOG(ERROR) << "error " << err << " with zipfile in unzReadCurrentFile";
          break;
        }
        if (err>0)
        if (fwrite(buf,err,1,fout)!=1)
        {
          LOG(ERROR) << "error in writing extracted file";
          err=UNZ_ERRNO;
          break;
        }
      }
      while (err>0);
      if (fout)
      fclose(fout);

      if (err==0)
      change_file_date(write_filename,file_info.dosDate,
            file_info.tmu_date);
    }

    if (err==UNZ_OK)
    {
      err = unzCloseCurrentFile (uf);
      if (err!=UNZ_OK)
      {
        LOG(ERROR) << "error " << err << " with zipfile in unzCloseCurrentFile";
      }
    }
    else
    unzCloseCurrentFile(uf); /* don't lose the error */
  }

  free(buf);
  return err;
}


int do_extract(unzFile uf,
      int opt_extract_without_path,
      int opt_overwrite,
      const char* password) {
  uLong i;
  unz_global_info64 gi;
  int err;

  err = unzGetGlobalInfo64(uf,&gi);
  if (err!=UNZ_OK) {
    LOG(ERROR) << "error " << err << " with zipfile in unzGetGlobalInfo";
  }

  for (i=0;i<gi.number_entry;i++)
  {
    if (do_extract_currentfile(uf,&opt_extract_without_path,
            &opt_overwrite,
            password) != UNZ_OK)
    break;

    if ((i+1)<gi.number_entry)
    {
      err = unzGoToNextFile(uf);
      if (err != UNZ_OK)
      {
        LOG(ERROR) << "error " << err << " with zipfile in unzGoToNextFile";
        break;
      }
    }
  }
  return 0;
}

int do_extract_onefile(unzFile uf,
      const char* filename,
      int opt_extract_without_path,
      int opt_overwrite,
      const char* password) {
  if (unzLocateFile(uf,filename,CASESENSITIVITY) != UNZ_OK) {
    LOG(ERROR) << "file " << filename << " not found in the zipfile";
    return 2;
  }

  if (do_extract_currentfile(uf,&opt_extract_without_path,
          &opt_overwrite,
          password) == UNZ_OK) {
    return 0;
  } else {
    return 1;
  }
}
