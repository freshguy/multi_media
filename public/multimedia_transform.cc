// Copyright (c) 2013 Juyou Inc. All rights reserved.
// Author: wangjuntao@juyou.com (Juntao Wang)

#include <sys/types.h>
#include <sys/wait.h>
#include <cxcore.h>
#include <highgui.h>
#include "base/time.h"
#include "base/string_util.h"
#include "file/file.h"
#include "juyou/base/base64/ZBase64.h"
#include "juyou/circles/public/utility.h"
#include "juyou/multi_media/public/multimedia_transform.h"

namespace juyou {
namespace multimedia {

cv::Ptr<IplImage> FIBITMAP2IplImage(FIBITMAP* fiBmp,
      const FREE_IMAGE_FORMAT& fif) {
  if (NULL == fiBmp || FIF_GIF != fif) {
    LOG(ERROR) << "NULL fiBmp or NOT a GIF IMAGE";
    return NULL;
  }
  int width  = FreeImage_GetWidth(fiBmp);
  int height = FreeImage_GetHeight(fiBmp);
  if (width <= 0 || height <= 0) {
    LOG(ERROR) << "Invalid width or height "
      << "with width = " << width << ", height = " << height;
    return NULL;
  }
  RGBQUAD* ptrPalette = FreeImage_GetPalette(fiBmp);
  BYTE intens;
  BYTE* pIntensity = &intens;
  cv::Ptr<IplImage> pImg = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 3);
  pImg->origin = 1;
  for (int i = 0; i < height; i++) {
    char* ptrDataLine = pImg->imageData + i * pImg->widthStep;
    for (int j = 0; j < width; j++) {
      FreeImage_GetPixelIndex(fiBmp, j, i, pIntensity);
      ptrDataLine[3 * j] = ptrPalette[intens].rgbBlue;
      ptrDataLine[3 * j + 1] = ptrPalette[intens].rgbGreen;
      ptrDataLine[3 * j + 2] = ptrPalette[intens].rgbRed;
    }
  }
  return pImg;
}

bool Gif2IplImages(const char* data,
      const size_t dataSize,
      std::vector<cv::Ptr<IplImage> >& mutilImgs) {
  if (data == NULL || dataSize == 0) {
    LOG(ERROR) << "NULL data when Gif2IplImages";
    return false;
  }
  FreeImage_Initialise(); // Must Initial freeimage
  FIMEMORY* memory = FreeImage_OpenMemory((BYTE*)data, dataSize);
  if (NULL == memory) {
    LOG(ERROR) << "FreeImage_OpenMemory error when Gif2IplImages";
    FreeImage_DeInitialise();
    return false;
  }
  
  FREE_IMAGE_FORMAT fif = FreeImage_GetFileTypeFromMemory(memory);
  if (FIF_UNKNOWN == fif) {
    LOG(ERROR) << "Error, Unknown image type when Gif2IplImages";
    if (NULL != memory) {
      FreeImage_CloseMemory(memory);
    }
    memory = NULL;
    FreeImage_DeInitialise();
    return false;
  } else if (FIF_GIF != fif) {  // Not a GIF 
    LOG(ERROR) << "Error, the image isn't a GIF when Gif2IplImages";
    if (NULL != memory) {
      FreeImage_CloseMemory(memory);
    }
    memory = NULL;
    FreeImage_DeInitialise();
    return false;
  }

  FIMULTIBITMAP* fiBmps = FreeImage_LoadMultiBitmapFromMemory(
        fif, memory, GIF_DEFAULT); // Decode the GIF file
  if (NULL == fiBmps) {
    LOG(ERROR) << "NULL fiBmps when Gif2IplImages";
    if (NULL != memory) {
      FreeImage_CloseMemory(memory);
    }
    memory = NULL;
    FreeImage_DeInitialise();
    return false;
  }

  int num = FreeImage_GetPageCount(fiBmps); // Obtain multiple frames
  VLOG(4) << "=== Frame count: " << num;
  for(int i = 0; i <= num; i++) {
    FIBITMAP* fiBmp = FreeImage_LockPage(fiBmps, i);
    if (fiBmp) {
      /// Transform the decoded Image data to opencv::IplImage
      cv::Ptr<IplImage> pImg =  FIBITMAP2IplImage(fiBmp, fif);
      FreeImage_UnlockPage(fiBmps, fiBmp, false);
      if (NULL == pImg) {
        LOG(ERROR) << "NULL IplImage for FIBITMAP_2_IplImage when Gif2IplImages";
        if (NULL != memory) {
          FreeImage_CloseMemory(memory);
        }
        memory = NULL;
        if (NULL != fiBmps) {
          FreeImage_CloseMultiBitmap(fiBmps, GIF_DEFAULT);
        }
        fiBmps = NULL;
        FreeImage_DeInitialise();
        return false;
      }
      mutilImgs.push_back(pImg);
    }
  }
  if (NULL != memory) {
    FreeImage_CloseMemory(memory);
  }
  memory = NULL;
  if (NULL != fiBmps) {
    FreeImage_CloseMultiBitmap(fiBmps, GIF_DEFAULT);
  }
  fiBmps = NULL;
  FreeImage_DeInitialise();
  return true;
}

bool ExtractActualRemoteFileInfo(
      const std::string remoteFileContent,
      std::string& typeName,
      std::string& origLocalFileName,
      std::string& actualRemoteFileContent) {
  if (remoteFileContent.empty()) {
    LOG(ERROR) << "Empty remoteFileContent when ExtractActualRemoteFileInfo";
    return false;
  }
  int i = 0;
  int start, end;
  int segmentCnt = 0;
  while (true) {
    if ('['  == remoteFileContent[i]) {
      start = i;
      while (']' != remoteFileContent[i] && i < remoteFileContent.length()) {
        i++;
      }
      if (']' == remoteFileContent[i]) {
        end = i;
        segmentCnt++;
      }
      if (1 == segmentCnt) {  // The first remoteFileContent prefix segment, which is the localFileName
        origLocalFileName = remoteFileContent.substr(
              start + 1, end - start - 1);
        VLOG(4) << "localFileName: " << origLocalFileName;
        /// Get the localFile's fileExtension from localFileName
        int extentionFlagPos
          = origLocalFileName.rfind('.') + 1;
        typeName = origLocalFileName.substr(extentionFlagPos);
        VLOG(4) << "extentionName: " << typeName;
      }
    }
    if (segmentCnt < 2) {
      i++;
    } else {
      break;
    }
  }

  /// Extract the remoteFile's actualContent
  int actualContentStart = end + 1;
  std::string encodeFileContent = remoteFileContent.substr(actualContentStart);

  /// Decode by Base64 Algorithm
  int decodeOutByte = 0;
  std::string decodeFileContent = ZBase64::Decode(
        encodeFileContent.c_str(), encodeFileContent.length(),
        decodeOutByte);
  VLOG(4) << "RemoteFileActualContent decodeOutByte: " << decodeOutByte;
  
  /// Trim the decodeFileContent end character. Because this ZBase64
  /// algorithm introduced the decodeFileContent end: "^O^O^O"
  /// May be this ZBase64's bug
  if ("gif" == typeName) {
#if 0
    actualRemoteFileContent = decodeFileContent.substr(0,
          decodeFileContent.length() - 4);  // trim the end 4 characters
#endif
    actualRemoteFileContent = decodeFileContent;
  } else {
#if 0
    actualRemoteFileContent = decodeFileContent.substr(0,
          decodeFileContent.length() - 3);  // trim the end 3 characters
#endif
    actualRemoteFileContent = decodeFileContent;
  }
  return true;
}

bool ZipFileContentUnzip(const std::string& zipFileContent,
      const std::string& zipFileNamePrefix,
      std::vector<std::string>& extractFileNames,
      std::string zipExtractPathPrefix) {
  if (zipFileContent.empty()) {
    LOG(ERROR) << "Empty zipFileContent when ZipFileContentUnzip";
    return false;
  }
  if (zipFileNamePrefix.empty()) {
    LOG(ERROR) << "Empty zipFileNamePrefix when ZipFileContentUnzip";
    return false;
  }
  if (zipExtractPathPrefix.empty()) {
    LOG(ERROR) << "Empty zipExtractPathPrefix when ZipFileContentUnzip";
    return false;
  }
  /// ***** Write Zip file *****
  std::string actualZipFilePath
    = zipExtractPathPrefix + "/" + zipFileNamePrefix + ".zip";
  if (!file::File::WriteStringToFile(zipFileContent,
          actualZipFilePath)) {
    LOG(ERROR) << "WriteStringToFile error for " << actualZipFilePath
      << " when ZipFileContentUnzip";
    return false;
  }
  
  /*
   * ***** Unzip Zip file *****
   */
  unzFile uf = NULL;
  if ((uf = unzOpen64(actualZipFilePath.c_str())) == NULL) {
    LOG(ERROR) << "Open Zip file error for " << actualZipFilePath;
    unzClose(uf);
    uf = NULL;
    file::File::DeleteRecursively(actualZipFilePath);  // Delete the .zip
    return false;
  }
  /// 2. Extract files in zip package
  std::string extractDir
    = zipExtractPathPrefix + "/" + zipFileNamePrefix;
  if (0 != mymkdir(extractDir.c_str())) {
    LOG(ERROR) << "mkdir for extract zip file error for "
      << "zipExtractPathPrefix = " << zipExtractPathPrefix
      << ", zipFileNamePrefix = " << zipFileNamePrefix;
    unzClose(uf);
    uf = NULL;
    file::File::DeleteRecursively(actualZipFilePath);  // Delete the .zip
    return false;
  }
  
  char origWorkDir[256];
  getwd(origWorkDir);
  VLOG(5) << "origWorkDir = " << origWorkDir;
  
  if (chdir(extractDir.c_str())) {
    LOG(ERROR) << "chdir to " << extractDir << " error for "
      << "extractDir = " << extractDir
      << "zipExtractPathPrefix = " << zipExtractPathPrefix
      << ", zipFileNamePrefix = " << zipFileNamePrefix;
    unzClose(uf);
    uf = NULL;
    file::File::DeleteRecursively(extractDir);  // Delete the new create dir
    file::File::DeleteRecursively(actualZipFilePath);  // Delete the .zip
    return false;
  }
  VLOG(5) << "Have change extract dir to: " << extractDir;

  int opt_do_extract_withoutpath = 0;
  int opt_overwrite = 0;
  const char* password = NULL;
  /// Extract zip file to the extractDir
  int retValue = do_extract(uf, opt_do_extract_withoutpath,
        opt_overwrite, password);
  unzClose(uf);
  uf = NULL;
  file::File::DeleteRecursively(actualZipFilePath);  // Delete the .zip

  /// Revert the initial state
  chdir(origWorkDir);              // revert to the original word-Dir
  getwd(origWorkDir);
  VLOG(5) << "Revert the work dir to: " << origWorkDir;
  
  file::File::GetFilesInDir(extractDir, &extractFileNames);
  return true;
}

bool ZipFileContentUnzip(const std::string& zipFileContent,
      const std::string& zipFileNamePrefix,
      const std::string& videoFileName,
      VideoFramesInfo& videoFramesInfo,
      std::map<std::string, std::string>& fileNameContentsMap,
      std::string pathPrefix) {
  if (zipFileContent.empty()) {
    LOG(ERROR) << "Empty zipFileContent when ZipFileContentUnZip";
    return false;
  }
  if (zipFileNamePrefix.empty()) {
    LOG(ERROR) << "Empty zipFileName when ZipFileContentUnzip";
    return false;
  }
  if (pathPrefix.empty()) {
    LOG(ERROR) << "Empty pathPrefix when ZipFileContentUnzip";
    return false;
  }
  /// Write Zip file
  std::string actualZipFilePath
    = pathPrefix + "/" + zipFileNamePrefix + ".zip";
  if (!file::File::WriteStringToFile(zipFileContent,
          actualZipFilePath)) {
    LOG(ERROR) << "WriteStringToFile error for " << actualZipFilePath
      << " when ZipFileContentUnzip";
    return false;
  }
  
  /// ***** Unzip Zip file *****
  unzFile uf = NULL;
  if ((uf = unzOpen64(actualZipFilePath.c_str())) == NULL) {
    LOG(ERROR) << "Open Zip file error for " << actualZipFilePath;
    unzClose(uf);
    uf = NULL;
    file::File::DeleteRecursively(actualZipFilePath);  // Delete the .zip
    return false;
  }

#if 0
  /*
   * If want fetch all the file names in the zip file,
   * uncomment this '#if 0' macro
   */
  /// 1. Fetch all the file names in the zip file
  std::vector<std::string> zipFileNames;
  if (!ExtractFileNamesInZip(uf, zipFileNames)) {
    LOG(ERROR) << "ExtractFileNamesInZip error for " << actualZipFilePath;
    unzClose(uf);
    return false;
  }

  /// For test
  for (int i = 0; i < zipFileNames.size(); i++) {
    VLOG(4) << i << "th filename_inzip is: " << zipFileNames[i];
  }

  if (UNZ_OK != unzGoToFirstFile(uf)) {
    LOG(ERROR) << "unzGoToFirstFile error when ZipFileContentUnzip "
      << "after ExtractFileNamesInZip";
  }
#endif
 
  /// 2. Extract files in zip package
  std::string extractDir = pathPrefix + "/" + zipFileNamePrefix;
  if (0 != mymkdir(extractDir.c_str())) {
    LOG(ERROR) << "mkdir for extract zip file error for "
      << "pathPrefix = " << pathPrefix
      << ", zipFileNamePrefix = " << zipFileNamePrefix;
    unzClose(uf);
    uf = NULL;
    file::File::DeleteRecursively(actualZipFilePath);  // Delete the .zip
    return false;
  }

  char origWorkDir[256];
  getwd(origWorkDir);
  VLOG(5) << "origWorkDir = " << origWorkDir;
  
  if (chdir(extractDir.c_str())) {
    LOG(ERROR) << "chdir to " << extractDir << " error for "
      << "extractDir = " << extractDir
      << "pathPrefix = " << pathPrefix
      << ", zipFileNamePrefix = " << zipFileNamePrefix;
    unzClose(uf);
    uf = NULL;
    file::File::DeleteRecursively(extractDir);  // Delete the new create dir
    file::File::DeleteRecursively(actualZipFilePath);  // Delete the .zip
    return false;
  }
  VLOG(5) << "Have change extract dir to: " << extractDir;

  int opt_do_extract_withoutpath = 0;
  int opt_overwrite = 0;
  const char* password = NULL;
  /// Extract zip file to the extractDir
  int retValue = do_extract(uf, opt_do_extract_withoutpath,
        opt_overwrite, password);
  unzClose(uf);
  uf = NULL;
  file::File::DeleteRecursively(actualZipFilePath);

  /// Revert the initial state
  chdir(origWorkDir);                         // revert to the original word-Dir
  getwd(origWorkDir);
  VLOG(5) << "Revert the work dir to: " << origWorkDir;
  std::vector<std::string> extractFilePaths;
  file::File::GetFilesInDir(extractDir, &extractFilePaths);
  bool done = true;
  for (int i = 0; i < extractFilePaths.size(); i++) {
    LOG(INFO) << i << "th extractFilePaths: " << extractFilePaths[i];
    std::string fileName = extractFilePaths[i].substr(
          extractFilePaths[i].rfind("/") + 1);
    if (fileName == videoFileName) {  // specially for video file
      if (!Video2FrameImages(extractFilePaths[i],
              videoFramesInfo)) {
        LOG(ERROR) << "Video2FrameImages error when ZipFileContentUnzip";
        file::File::DeleteRecursively(extractDir);  // Delete the zip extract folder
        return false;
      }
    } else {
      if (!file::File::ReadFileToString(extractFilePaths[i],
              &fileNameContentsMap[fileName])) {
        LOG(ERROR) << "Read file " << fileName << " error";
        done = false;
        break;
      }
    }
  }

  file::File::DeleteRecursively(extractDir);  // Delete the zip extract folder
  if (!done) {
    LOG(ERROR) << "ReadFileToString error(ImCompletely) when ZipFileContentUnzip "
      << "for " << actualZipFilePath;
    return false;
  }
  return true;
}

bool ExtractFileNamesInZip(unzFile uf,
      std::vector<std::string>& zipFileNames)  {
  uLong i = 0;
  unz_global_info64 gi;
  int err;

  err = unzGetGlobalInfo64(uf, &gi);
  if (UNZ_OK != err) {
    LOG(ERROR) << "error " << err
      << " with zipfile in unzGetGlobalInfo";
    return false;
  }

  for (i = 0; i < gi.number_entry; i++) {
    char filename_inzip[256];
    unz_file_info64 file_info;
    uLong ratio = 0;
    const char* string_method;
    char charCrypt = ' ';
    err = unzGetCurrentFileInfo64(uf, &file_info,
          filename_inzip, sizeof(filename_inzip),
          NULL, 0, NULL, 0);
    if (err!=UNZ_OK) {
      LOG(ERROR) << "error " << err
        << " with zipfile in unzGetCurrentFileInfo";
      return false;
    }
    zipFileNames.push_back(filename_inzip);

    if (file_info.uncompressed_size > 0) {
      ratio =
        (uLong)((file_info.compressed_size * 100) / file_info.uncompressed_size);
    }
    /* display a '*' if the file is crypted */
    if ((file_info.flag & 1) != 0) {
      charCrypt = '*';
    }

    if (file_info.compression_method == 0) {
      string_method = "Stored";
    } else if (file_info.compression_method == Z_DEFLATED) {
      uInt iLevel = (uInt)((file_info.flag & 0x6) / 2);
      if (iLevel == 0) {
        string_method = "Defl:N";
      } else if (iLevel == 1) {
        string_method = "Defl:X";
      } else if ((iLevel == 2) || (iLevel == 3)) {
        string_method = "Defl:F"; /* 2:fast , 3 : extra fast*/
      }
    } else if (file_info.compression_method == Z_BZIP2ED) {
      string_method = "BZip2 ";
    } else {
      string_method = "Unkn. ";
    }

    Display64BitsSize(file_info.uncompressed_size, 7);
    printf("  %6s%c", string_method, charCrypt);
    Display64BitsSize(file_info.compressed_size, 7);
    printf(" %3lu%%  %2.2lu-%2.2lu-%2.2lu  %2.2lu:%2.2lu  %8.8lx   %s\n",
          ratio,
          (uLong)file_info.tmu_date.tm_mon + 1,
          (uLong)file_info.tmu_date.tm_mday,
          (uLong)file_info.tmu_date.tm_year % 100,
          (uLong)file_info.tmu_date.tm_hour, (uLong)file_info.tmu_date.tm_min,
          (uLong)file_info.crc, filename_inzip);
    if ((i+1) < gi.number_entry) {
      err = unzGoToNextFile(uf);
      if (err != UNZ_OK) {
        LOG(ERROR) << "error " << err << " with zipfile in unzGoToNextFile";
        return false;
      }
    }
  } 
  return true;
}

bool Video2FrameImages(const std::string& videoFileName,
      VideoFramesInfo& videoFramesInfo) {
  CvCapture* capture = NULL;
  /// CvCapture* capture = cvCreateFileCapture(videoFileName.c_str());
  //if ((capture = cvCaptureFromFile(videoFileName.c_str())) == NULL) {
  if ((capture = cvCreateFileCapture(videoFileName.c_str())) == NULL) {
    LOG(ERROR) << "Cann't open video file: " << videoFileName;
    return false;
  }
  //cvSetCaptureProperty(capture, CV_CAP_PROP_POS_MSEC, 10);  // 12.25
  videoFramesInfo.frameH_ = (int)cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT);
  videoFramesInfo.frameW_ = (int)cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH);
  videoFramesInfo.fps_ = (int)cvGetCaptureProperty(capture, CV_CAP_PROP_FPS);
  videoFramesInfo.frameCnt_ =(int)cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_COUNT);
  
  base::TimeTicks wstartT = base::TimeTicks::HighResNow();
  IplImage* frameImg = NULL;
#if 0 
  int i = 0;
  while (1) {
    /// It seems the cvQueryFrame wait some signal after
    /// query the Final frame, which will consume a lot time
    base::TimeTicks startT = base::TimeTicks::HighResNow();
    frameImg = cvQueryFrame(capture);
    if (!frameImg) {
      break;
    }
    base::TimeTicks endT = base::TimeTicks::HighResNow();
    base::TimeDelta duration = endT - startT;
    VLOG(4) << "queryFrame consume " << duration.InMilliseconds() << "ms for frame " << i;
    IplImage* frameImgClone = cvCloneImage(frameImg);
    cvSaveImage(std::string("/tmp/test/" + IntToString(i++) + ".jpg").c_str(), frameImgClone);
    videoFramesInfo.frameImgs_.push_back(frameImgClone);
  }
#endif

  for (int i = 0; i < videoFramesInfo.frameCnt_; i++) {
    base::TimeTicks startT = base::TimeTicks::HighResNow();
    frameImg = cvQueryFrame(capture);
    if (!frameImg) {
      break;
    }
    base::TimeTicks endT = base::TimeTicks::HighResNow();
    base::TimeDelta duration = endT - startT;
    VLOG(5) << "queryFrame consume "
      << duration.InMilliseconds() << "ms for frame " << i + 1;
    IplImage* frameImgClone = cvCloneImage(frameImg);
    // cvSaveImage(std::string("/tmp/test/" + IntToString(i) + ".jpg").c_str(), frameImgClone);
    videoFramesInfo.frameImgs_.push_back(frameImgClone);
  }

  base::TimeTicks wendT = base::TimeTicks::HighResNow();
  base::TimeDelta wduration = wendT - wstartT;
  VLOG(4) << "while loop consume "
    << wduration.InMilliseconds() << "ms";

  cvReleaseCapture(&capture);  // free CvCapture
  VLOG(4) << "frameW = " << videoFramesInfo.frameW_
    << ", frameH " << videoFramesInfo.frameH_
    << ", fps = " << videoFramesInfo.fps_
    << ", frameCnt = " << videoFramesInfo.frameCnt_;
  return true;
}

bool Video2FrameImagesFO(const std::string& videoFileName,
      VideoFramesInfo& videoFramesInfo,
      std::string framesParentDir) {
  CvCapture* capture = NULL;
  if ((capture = cvCreateFileCapture(videoFileName.c_str())) == NULL) {
    LOG(ERROR) << "Cann't open video file: " << videoFileName;
    return false;
  }
  
  videoFramesInfo.frameH_ = (int)cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT);
  videoFramesInfo.frameW_ = (int)cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH);
  videoFramesInfo.fps_ = (int)cvGetCaptureProperty(capture, CV_CAP_PROP_FPS);
  cvReleaseCapture(&capture);  // free CvCapture

  std::string framesDir = framesParentDir + "/" + Utility::GetUuid("videoFrames_");
  if (!file::File::CreateDir(framesDir, 0777)) {
    LOG(ERROR) << "Create frames dir error with "
      << "framesDir: " << framesDir
      << ", videoFileName: " << videoFileName;
    return false;
  }
  /// Get the video's 'rotate' metadata
  std::string getRotateCommand = "ffmpeg -i " + videoFileName;
  getRotateCommand += " 2>&1|grep 'rotate'|awk -F\" \" '{print $NF}'";
  VLOG(4) << "GetRotateCommand: " << getRotateCommand;
  int resultLen = 50;
  char rotateResult[resultLen];
  if (0 != mysystem(getRotateCommand.c_str(), rotateResult, resultLen)) {
    LOG(ERROR) << "Get Video MediaInfo for 'rotate error "
      << "with videoFileName = " << videoFileName;
    file::File::DeleteRecursively(framesDir);
    return false;
  }
  /// The rotateDegree is an CounterClockwise
  int rotateDegree = StringToInt(std::string(rotateResult));
  VLOG(4) << "Video rotate degree: " << rotateDegree;
  std::string transposeVideoFileName;
  if (90 == rotateDegree) {
    /// 1 = 90Clockwise
    transposeVideoFileName = framesDir + "/rotate90Video.mp4";
    std::string rotate90Cmd = "ffmpeg -i " + videoFileName;
    rotate90Cmd += " -vf \"transpose=1\" -strict -2 ";
    rotate90Cmd += transposeVideoFileName;
    VLOG(4) << "rotateDegree = " << rotateDegree
      << ".rotate90Cmd: " << rotate90Cmd;
    if (system(rotate90Cmd.c_str()) == -1) {
      LOG(ERROR) << "ffmpeg transpose error when Video2ImagesFO with "
        << "videoFileName: " << videoFileName
        << ", rotateDegree: " << rotateDegree;
      file::File::DeleteRecursively(framesDir);
      return false;
    }
  } else if (180 == rotateDegree) {
    /*
     * Twice 1 = 90Clockwise
     */
    /// first 1 = 90Clockwise
    std::string first90rotateFile = framesDir + "/rotate180Video1.mp4";
    std::string rotate90Cmd1 = "ffmpeg -i " + videoFileName;
    rotate90Cmd1 += " -vf \"transpose=1\" -strict -2 ";
    rotate90Cmd1 += first90rotateFile;
    VLOG(4) << "rotateDegree = " << rotateDegree
      << ".rotate90Cmd1: " << rotate90Cmd1;
    if (system(rotate90Cmd1.c_str()) == -1) {
      LOG(ERROR) << "ffmpeg transpose error when Video2ImageFO with "
        << "videoFileName: " << videoFileName
        << ", rotateDegree: " << rotateDegree << ". FIRST";
      file::File::DeleteRecursively(framesDir);
      return false;
    }
    /// second 1 = 90Clockwise
    std::string second90rotateFile = framesDir + "/rotate180Video2.mp4";
    std::string rotate90Cmd2 = "ffmpeg -i " + first90rotateFile;
    rotate90Cmd2 += " -vf \"transpose=1\" -strict -2 ";
    rotate90Cmd2 += second90rotateFile;
    VLOG(4) << "rotateDegree = " << rotateDegree
      << ".rotate90Cmd2: " << rotate90Cmd2;
    if (system(rotate90Cmd2.c_str()) == -1) {
      LOG(ERROR) << "ffmpeg transpose error when Video2ImageFO with "
        << "first90rotateFile: " << first90rotateFile
        << ", rotateDegree: " << rotateDegree << ". SECOND";
      file::File::DeleteRecursively(framesDir);
      return false;
    }
    transposeVideoFileName = second90rotateFile;
  } else if (270 == rotateDegree) {
    /// 2 = 90CounterClockwise
    transposeVideoFileName = framesDir + "/rotate270Video.mp4";
    std::string rotate270Cmd = "ffmpeg -i " + videoFileName;
    rotate270Cmd += " -vf \"transpose=2\" -strict -2 ";
    rotate270Cmd += transposeVideoFileName;
    VLOG(4) << "rotateDegree = " << rotateDegree
      << ".rotate270Cmd: " << rotate270Cmd;
    if (system(rotate270Cmd.c_str()) == -1) {
      LOG(ERROR) << "ffmpeg tranpose error when Video2ImageFO with "
        << "videoFileName: " << videoFileName
        << ", rotateDegree: " << rotateDegree;
      file::File::DeleteRecursively(framesDir);
      return false;
    }
  } else {
    transposeVideoFileName = videoFileName;
  }
  /// "ffmpeg -i ../20131219134623.mp4 %d.jpg"
  std::string decodeFramesCommand = "ffmpeg -i ";
  decodeFramesCommand += transposeVideoFileName;  // videoFileName;
  decodeFramesCommand += " ";
  decodeFramesCommand += framesDir;
  decodeFramesCommand += "/%d.jpg";
  VLOG(4) << "decodeFramesCommand: " << decodeFramesCommand;
  if (system(decodeFramesCommand.c_str()) == -1) {
    LOG(ERROR) << "ffmepg decode frames error when Video2ImagesFO with "
      << "framesDir: " << framesDir
      << ", videoFileName: " << transposeVideoFileName  // videoFileName
      << ", decodeFramesCommand: " << decodeFramesCommand;
    file::File::DeleteRecursively(framesDir);
    return false;
  }

  std::vector<std::string> frameNames;
  if (!file::File::GetFilesInDir(framesDir,
          &frameNames)) {
    LOG(ERROR) << "Get all frames fileName error when Video2ImagesFO with "
      << "framesDir: " << framesDir
      << ", videoFileName: " << transposeVideoFileName;  // videoFileName;
    file::File::DeleteRecursively(framesDir);
    return false;
  }
  std::sort(frameNames.begin(), frameNames.end(), FramesSortByFileNameASC);
  int frameCnt = 0;
  for (int i = 0; i < frameNames.size(); i++) {
    if (frameNames[i].substr(frameNames[i].rfind('.') + 1) == "jpg") {
      // VLOG(4) << i << "th frameName: " << frameNames[i];
      IplImage* frameImg = cvLoadImage(frameNames[i].c_str());
      videoFramesInfo.frameImgs_.push_back(frameImg);
      frameCnt++;
    }
  }
  videoFramesInfo.frameCnt_ = frameCnt;
  VLOG(5) << transposeVideoFileName  // videoFileName
    << "'s frameCnt is " << videoFramesInfo.frameCnt_;
  /// Delete the tmp frames dir
  file::File::DeleteRecursively(framesDir);
  return true;
}

bool FramesSortByFileNameASC(const std::string& frameFileName1,
      const std::string& frameFileName2) {
  int frameIndexStart = frameFileName1.rfind('/') + 1;
  int frameIndexEnd = frameFileName1.rfind('.') - 1;
  int frameIndex1 = StringToInt(frameFileName1.substr(
          frameIndexStart, frameIndexEnd - frameIndexStart + 1));
  
  frameIndexStart = frameFileName2.rfind('/') + 1;
  frameIndexEnd = frameFileName2.rfind('.') - 1;
  int frameIndex2 = StringToInt(frameFileName2.substr(
          frameIndexStart, frameIndexEnd - frameIndexStart + 1));
  return frameIndex1 < frameIndex2;
}

void Image_to_video() {
  LOG(INFO) << "------------- image to video ... ----------------n";
  //初始化视频编写器，参数根据实际视频文件修改
  CvVideoWriter *writer = 0;
  int isColor = 1;
  int fps     = 30; // or 25
  int frameW = 480; // 744 for firewire cameras
  int frameH = 800; // 480 for firewire cameras
  writer=cvCreateVideoWriter("/home/taogle/Desktop/gif/frames/good.avi",  // only for avi
        //CV_FOURCC('X','V','I','D'),
        CV_FOURCC('D','I','V','X'),
        fps,
        cvSize(frameW,frameH),
        isColor);
  LOG(INFO) << "tvideo height: " << frameH << ", width: " << frameW << ", fps: " << fps;
  int i = 0;
  IplImage* img = NULL;
  while(i < 251) {
    std::string imgName = "/home/taogle/Desktop/gif/frames/" + IntToString(i + 1) + ".jpg";
    img = cvLoadImage(imgName.c_str());
    if(!img) {
      LOG(ERROR) << "Could not load image file: " << imgName;
      return;
    }  
    cvWriteFrame(writer, img);
    cvReleaseImage(&img);
  }  
}



int mysystem(const char* cmdstring, char* buf, int len) {
  int fd[2]; pid_t pid;
  int n, count;
  memset(buf, 0, len);
  if (pipe(fd) < 0) {
    return -1;
  }
  if ((pid = fork()) < 0) {
    return -1;
  } else if (pid > 0) {    // parent process
    VLOG(4) << "Parent pid: " << pid;
    close(fd[1]);          // close write end
    count = 0;
    while ((n = read(fd[0], buf + count, len)) > 0 && count > len) {
      count += n;
    }
    close(fd[0]);
    int waitStatus = 0;
    if ((waitStatus = waitpid(pid, NULL, 0)) > 0) {
      VLOG(4) << "Have waited collect the child-process: "
        << waitStatus;
      return 0;
    } else if (-1 == waitStatus) {
      LOG(ERROR) << "waitpid error with pid = " << pid
        << ". ERRNO: " << strerror(errno);
      return -1;
    }
  } else {              // child process
    VLOG(4) << "This is the child process";
    close(fd[0]);       // close read end
    if (fd[1] != STDOUT_FILENO) {
      if (dup2(fd[1], STDOUT_FILENO) != STDOUT_FILENO) {
        LOG(ERROR) << "dup2 error";
        return -1;
      }
      close(fd[1]);
    }
    if (execl("/bin/sh", "sh", "-c", cmdstring, (char*)0) == -1) {
      LOG(ERROR) << "execl error";
      return -1;
    }
  }
  return 0;
}



#if 0
/// http://stackoverflow.com/questions/4248316/video-generation-using-ffmpeg?rq=1
(void)createVideoFromImages 
{
  NSString *fileName2 = [Utilities documentsPath:[NSString stringWithFormat:@"test.mov"]];
  NSLog(@"filename: %@",fileName2);

  //Register all formats and codecs

  AVCodec *codec;

  //avcodec_register_all();
  //avdevice_register_all();

  av_register_all();


  AVCodecContext *c= NULL; 
  int i, out_size, size, outbuf_size;
  FILE *f;
  AVFrame *picture;
  uint8_t *outbuf;

  printf("Video encoding\n");

  /* find the mpeg video encoder */
  codec = avcodec_find_encoder(CODEC_ID_MPEG2VIDEO);
  if (!codec) 
  {
    fprintf(stderr, "codec not found\n");
    exit(1);
  }

  c= avcodec_alloc_context();
  picture= avcodec_alloc_frame();

  /* put sample parameters */
  c->bit_rate = 400000;
  /* resolution must be a multiple of two */
  c->width = 256;
  c->height = 256;//258;

  /* frames per second */
  c->time_base= (AVRational){1,25};
  c->gop_size = 10; /* emit one intra frame every ten frames */
  c->max_b_frames=1;
  c->pix_fmt =  PIX_FMT_YUV420P;//PIX_FMT_YUV420P;

  /* open it */
  if (avcodec_open(c, codec) < 0) {
    fprintf(stderr, "could not open codec\n");
    exit(1);
  }

  const char* filename_cstr = [fileName2 cStringUsingEncoding:NSUTF8StringEncoding];
  f = fopen(filename_cstr, "wb");
  if (!f) {
    fprintf(stderr, "could not open %s\n", fileName2);
    exit(1);
  }

  /* alloc image and output buffer */
  outbuf_size = 100000;
  outbuf = malloc(outbuf_size);
  size = c->width * c->height;

  #pragma mark -
  AVFrame* outpic = avcodec_alloc_frame();
  int nbytes = avpicture_get_size(PIX_FMT_YUV420P, c->width, c->height);

  NSLog(@"bytes: %d",nbytes);

  //create buffer for the output image
  uint8_t* outbuffer = (uint8_t*)av_malloc(nbytes);

  for(i=100;i<104;i++) 
  {
    fflush(stdout);

    int numBytes = avpicture_get_size(PIX_FMT_YUV420P, c->width, c->height);
    NSLog(@"numBytes: %d",numBytes);
    uint8_t *buffer = (uint8_t *)av_malloc(numBytes*sizeof(uint8_t));

    UIImage *image;

    image = [UIImage imageWithContentsOfFile:[Utilities bundlePath:[NSString stringWithFormat:@"%d.png",i]]];


    /*
    if(i>=98)//for video images
    {
       NSLog(@"i: %d",i);
       image = [UIImage imageWithContentsOfFile:[Utilities documentsPath:[NSString stringWithFormat:@"image0098.png"]]]; ///@"image0098.png"]];
       //[NSString stringWithFormat:@"%d.png", i]];
    }
    else //for custom image
    {
       image = [UIImage imageWithContentsOfFile:[Utilities bundlePath:[NSString stringWithFormat:@"image%04d.png", i]]];
       //[UIImage imageNamed:[NSString stringWithFormat:@"%d.png", i]];//@"image%04d.png",i]];
    }*/

    CGImageRef newCgImage = [image CGImage];

    NSLog(@"No. of Bits per component: %d",CGImageGetBitsPerComponent([image CGImage]));
    NSLog(@"No. of Bits per pixel: %d",CGImageGetBitsPerPixel([image CGImage]));
    NSLog(@"No. of Bytes per row: %d",CGImageGetBytesPerRow([image CGImage]));


    CGDataProviderRef dataProvider = CGImageGetDataProvider(newCgImage);
    CFDataRef bitmapData = CGDataProviderCopyData(dataProvider);
    buffer = (uint8_t *)CFDataGetBytePtr(bitmapData);   

    struct SwsContext* fooContext;

    avpicture_fill((AVPicture*)picture, buffer, PIX_FMT_RGBA, c->width, c->height);

    avpicture_fill((AVPicture*)outpic, outbuffer, PIX_FMT_YUV420P, c->width, c->height);

    fooContext= sws_getContext(c->width, c->height, 
       PIX_FMT_RGBA, 
        c->width, c->height, 
        PIX_FMT_YUV420P, 
        SWS_FAST_BILINEAR , NULL, NULL, NULL);

//}

  //perform the conversion

  NSLog(@"linesize: %d", picture->linesize[0]);

  sws_scale(fooContext, picture->data, picture->linesize, 0, c->height, outpic->data, outpic->linesize);

  // Here is where I try to convert to YUV

  /* encode the image */
  out_size = avcodec_encode_video(c, outbuf, outbuf_size, outpic);
  printf("encoding frame %d (size=%d)\n", i, out_size);
  fwrite(outbuf, 1, out_size, f);

  NSLog(@"%d",sizeof(f));

  free(buffer);
  buffer = NULL;      

  }

  /* get the delayed frames */
  for( ; out_size; i++) 
  {
    fflush(stdout);
    out_size = avcodec_encode_video(c, outbuf, outbuf_size, NULL);
    printf("write frame %3d (size=%5d)\n", i, out_size);
    fwrite(outbuf, 1, outbuf_size, f);      
  }

  /* add sequence end code to have a real mpeg file */
  outbuf[0] = 0x00;
  outbuf[1] = 0x00;
  outbuf[2] = 0x01;
  outbuf[3] = 0xb7;
  fwrite(outbuf, 1, 4, f);

  fclose(f);
  free(outbuf);

  avcodec_close(c);
  av_free(c);
  av_free(picture);
  printf("\n");

}


AVCodec* codec;
AVCodecContext* c = NULL;
uint8_t* outbuf;
int i, out_size, outbuf_size;

avcodec_register_all();                                 // mandatory to register ffmpeg functions

printf("Video encoding\n");

codec = avcodec_find_encoder(CODEC_ID_H264);            // finding the H264 encoder
if (!codec) {
    fprintf(stderr, "Codec not found\n");
    exit(1);
}
else printf("H264 codec found\n");

c = avcodec_alloc_context3(codec);
c->bit_rate = 400000;
c->width = 1280;                                        // resolution must be a multiple of two (1280x720),(1900x1080),(720x480)
c->height = 720;
c->time_base.num = 1;                                   // framerate numerator
c->time_base.den = 25;                                  // framerate denominator
c->gop_size = 10;                                       // emit one intra frame every ten frames
c->max_b_frames = 1;                                    // maximum number of b-frames between non b-frames
c->keyint_min = 1;                                      // minimum GOP size
c->i_quant_factor = (float)0.71;                        // qscale factor between P and I frames
c->b_frame_strategy = 20;                               ///// find out exactly what this does
c->qcompress = (float)0.6;                              ///// find out exactly what this does
c->qmin = 20;                                           // minimum quantizer
c->qmax = 51;                                           // maximum quantizer
c->max_qdiff = 4;                                       // maximum quantizer difference between frames
c->refs = 4;                                            // number of reference frames
c->trellis = 1;                                         // trellis RD Quantization
c->pix_fmt = PIX_FMT_YUV420P;                           // universal pixel format for video encoding
c->codec_id = CODEC_ID_H264;
c->codec_type = AVMEDIA_TYPE_VIDEO;

if (avcodec_open2(c, codec,NULL) < 0) {
    fprintf(stderr, "Could not open codec\n");          // opening the codec
    exit(1);
}
else printf("H264 codec opened\n");

outbuf_size = 100000 + c->width*c->height*(32>>3);      // allocate output buffer
outbuf = static_cast<uint8_t *>(malloc(outbuf_size));
printf("Setting buffer size to: %d\n",outbuf_size);

FILE* f = fopen("example.mpg","wb");                    // opening video file for writing
if(!f) printf("x  -  Cannot open video file for writing\n");
else printf("Opened video file for writing\n");

// encode video
for(i=0;i<STREAM_FRAME_RATE*STREAM_DURATION;i++) {
    fflush(stdout);

    screenCap();                                                                                                // taking screenshot

    int nbytes = avpicture_get_size(PIX_FMT_YUV420P, c->width, c->height);                                      // allocating outbuffer
    uint8_t* outbuffer = (uint8_t*)av_malloc(nbytes*sizeof(uint8_t));

    AVFrame* inpic = avcodec_alloc_frame();                                                                     // mandatory frame allocation
    AVFrame* outpic = avcodec_alloc_frame();

    outpic->pts = (int64_t)((float)i * (1000.0/((float)(c->time_base.den))) * 90);                              // setting frame pts
    avpicture_fill((AVPicture*)inpic, (uint8_t*)pPixels, PIX_FMT_RGB32, c->width, c->height);                   // fill image with input screenshot
    avpicture_fill((AVPicture*)outpic, outbuffer, PIX_FMT_YUV420P, c->width, c->height);                        // clear output picture for buffer copy
    av_image_alloc(outpic->data, outpic->linesize, c->width, c->height, c->pix_fmt, 1); 

    inpic->data[0] += inpic->linesize[0]*(screenHeight-1);                                                      // flipping frame
    inpic->linesize[0] = -inpic->linesize[0];                                                                   // flipping frame

    struct SwsContext* fooContext = sws_getContext(screenWidth, screenHeight, PIX_FMT_RGB32, c->width, c->height, PIX_FMT_YUV420P, SWS_FAST_BILINEAR, NULL, NULL, NULL);
    sws_scale(fooContext, inpic->data, inpic->linesize, 0, c->height, outpic->data, outpic->linesize);          // converting frame size and format

    out_size = avcodec_encode_video(c, outbuf, outbuf_size, outpic);                                            // encoding video
    printf("Encoding frame %3d (size=%5d)\n", i, out_size);
    fwrite(outbuf, 1, out_size, f);

    delete [] pPixels;                                                                                          // freeing memory
    av_free(outbuffer);     
    av_free(inpic);
    av_free(outpic);
}

for(; out_size; i++) {
    fflush(stdout);

    out_size = avcodec_encode_video(c, outbuf, outbuf_size, NULL);                                              // encode the delayed frames
    printf("Writing frame %3d (size=%5d)\n", i, out_size);
    fwrite(outbuf, 1, out_size, f);
}

outbuf[0] = 0x00;
outbuf[1] = 0x00;                                                                                               // add sequence end code to have a real mpeg file
outbuf[2] = 0x01;
outbuf[3] = 0xb7;
fwrite(outbuf, 1, 4, f);
fclose(f);

avcodec_close(c);                                                                                               // freeing memory
free(outbuf);
av_free(c);
printf("Closed codec and Freed\n");
#endif


#if 0
/*
 * Video encoding example
 */
static void video_encode_example(const char *filename)
{
    AVCodec *codec;
    AVCodecContext *c= NULL;
    int i, out_size, size, x, y, outbuf_size;
    FILE *f;
    AVFrame *picture;
    uint8_t *outbuf, *picture_buf;

    printf("Video encoding\n");

    /* find the mpeg1 video encoder */
    codec = avcodec_find_encoder(CODEC_ID_MPEG1VIDEO);
    if (!codec) {
        fprintf(stderr, "codec not found\n");
        exit(1);
    }

    c= avcodec_alloc_context();
    picture= avcodec_alloc_frame();

    /* put sample parameters */
    c->bit_rate = 400000;
    /* resolution must be a multiple of two */
    c->width = 352;
    c->height = 288;
    /* frames per second */
    c->time_base= (AVRational){1,25};
    c->gop_size = 10; /* emit one intra frame every ten frames */
    c->max_b_frames=1;
    c->pix_fmt = PIX_FMT_YUV420P;

    /* open it */
    if (avcodec_open(c, codec) < 0) {
        fprintf(stderr, "could not open codec\n");
        exit(1);
    }

    f = fopen(filename, "wb");
    if (!f) {
        fprintf(stderr, "could not open %s\n", filename);
        exit(1);
    }

    /* alloc image and output buffer */
    outbuf_size = 100000;
    outbuf = malloc(outbuf_size);
    size = c->width * c->height;
    picture_buf = malloc((size * 3) / 2); /* size for YUV 420 */

    picture->data[0] = picture_buf;
    picture->data[1] = picture->data[0] + size;
    picture->data[2] = picture->data[1] + size / 4;
    picture->linesize[0] = c->width;
    picture->linesize[1] = c->width / 2;
    picture->linesize[2] = c->width / 2;

    /* encode 1 second of video */
    for(i=0;i<25;i++) {
        fflush(stdout);
        /* prepare a dummy image */
        /* Y */
        for(y=0;y<c->height;y++) {
            for(x=0;x<c->width;x++) {
                picture->data[0][y * picture->linesize[0] + x] = x + y + i * 3;
            }
        }

        /* Cb and Cr */
        for(y=0;y<c->height/2;y++) {
            for(x=0;x<c->width/2;x++) {
                picture->data[1][y * picture->linesize[1] + x] = 128 + y + i * 2;
                picture->data[2][y * picture->linesize[2] + x] = 64 + x + i * 5;
            }
        }

        /* encode the image */
        out_size = avcodec_encode_video(c, outbuf, outbuf_size, picture);
        printf("encoding frame %3d (size=%5d)\n", i, out_size);
        fwrite(outbuf, 1, out_size, f);
    }

    /* get the delayed frames */
    for(; out_size; i++) {
        fflush(stdout);

        out_size = avcodec_encode_video(c, outbuf, outbuf_size, NULL);
        printf("write frame %3d (size=%5d)\n", i, out_size);
        fwrite(outbuf, 1, out_size, f);
    }

    /* add sequence end code to have a real mpeg file */
    outbuf[0] = 0x00;
    outbuf[1] = 0x00;
    outbuf[2] = 0x01;
    outbuf[3] = 0xb7;
    fwrite(outbuf, 1, 4, f);
    fclose(f);
    free(picture_buf);
    free(outbuf);

    avcodec_close(c);
    av_free(c);
    av_free(picture);
    printf("\n");
}
#endif

#if 0
http://stackoverflow.com/questions/3334939/encoding-images-to-video-with-ffmpeg

  // Register all formats and codecs
    av_register_all();

    AVCodec *codec;
    AVCodecContext *c= NULL;
    int i, out_size, size, outbuf_size;
    FILE *f;
    AVFrame *picture;
    uint8_t *outbuf;

    printf("Video encoding\n");

    /* find the mpeg video encoder */
    codec = avcodec_find_encoder(CODEC_ID_MPEG2VIDEO);
    if (!codec) {
        fprintf(stderr, "codec not found\n");
        exit(1);
    }

    c= avcodec_alloc_context();
    picture= avcodec_alloc_frame();

    /* put sample parameters */
    c->bit_rate = 400000;
    /* resolution must be a multiple of two */
    c->width = 352;
    c->height = 288;
    /* frames per second */
    c->time_base= (AVRational){1,25};
    c->gop_size = 10; /* emit one intra frame every ten frames */
    c->max_b_frames=1;
    c->pix_fmt = PIX_FMT_YUV420P;

    /* open it */
    if (avcodec_open(c, codec) < 0) {
        fprintf(stderr, "could not open codec\n");
        exit(1);
    }

    f = fopen(filename, "wb");
    if (!f) {
        fprintf(stderr, "could not open %s\n", filename);
        exit(1);
    }

    /* alloc image and output buffer */
    outbuf_size = 100000;
    outbuf = malloc(outbuf_size);
    size = c->width * c->height;

#pragma mark -
    AVFrame* outpic = avcodec_alloc_frame();
    int nbytes = avpicture_get_size(PIX_FMT_YUV420P, c->width, c->height);

    //create buffer for the output image
    uint8_t* outbuffer = (uint8_t*)av_malloc(nbytes);

#pragma mark -  
    for(i=1;i<77;i++) {
        fflush(stdout);

        int numBytes = avpicture_get_size(PIX_FMT_YUV420P, c->width, c->height);
        uint8_t *buffer = (uint8_t *)av_malloc(numBytes*sizeof(uint8_t));

        UIImage *image = [UIImage imageNamed:[NSString stringWithFormat:@"10%d", i]];
        CGImageRef newCgImage = [image CGImage];

        CGDataProviderRef dataProvider = CGImageGetDataProvider(newCgImage);
        CFDataRef bitmapData = CGDataProviderCopyData(dataProvider);
        buffer = (uint8_t *)CFDataGetBytePtr(bitmapData);   

        avpicture_fill((AVPicture*)picture, buffer, PIX_FMT_RGB8, c->width, c->height);
        avpicture_fill((AVPicture*)outpic, outbuffer, PIX_FMT_YUV420P, c->width, c->height);

        struct SwsContext* fooContext = sws_getContext(c->width, c->height, 
                                                PIX_FMT_RGB8, 
                                                c->width, c->height, 
                                                PIX_FMT_YUV420P, 
                                                SWS_FAST_BILINEAR, NULL, NULL, NULL);

        //perform the conversion
        sws_scale(fooContext, picture->data, picture->linesize, 0, c->height, outpic->data, outpic->linesize);
        // Here is where I try to convert to YUV

        /* encode the image */
        out_size = avcodec_encode_video(c, outbuf, outbuf_size, outpic);
        printf("encoding frame %3d (size=%5d)\n", i, out_size);
        fwrite(outbuf, 1, out_size, f);

        free(buffer);
        buffer = NULL;      

    }

    /* get the delayed frames */
    for(; out_size; i++) {
        fflush(stdout);

        out_size = avcodec_encode_video(c, outbuf, outbuf_size, NULL);
        printf("write frame %3d (size=%5d)\n", i, out_size);
        fwrite(outbuf, 1, outbuf_size, f);      
    }

    /* add sequence end code to have a real mpeg file */
    outbuf[0] = 0x00;
    outbuf[1] = 0x00;
    outbuf[2] = 0x01;
    outbuf[3] = 0xb7;
    fwrite(outbuf, 1, 4, f);
    fclose(f);
    free(outbuf);

    avcodec_close(c);
    av_free(c);
    av_free(picture);
    printf("\n");

#endif

}  // namespace multimedia
}  // namespace juyou
