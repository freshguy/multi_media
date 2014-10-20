// Copyright (c) 2013 Juyou Inc. All rights reserved.
// Author: wangjuntao@juyou.com (Juntao Wang)

#ifndef JUYOU_MULTI_MEDIA_PUBLIC_GIF_DECODER_H_
#define JUYOU_MULTI_MEDIA_PUBLIC_GIF_DECODER_H_

#include "opencv2/opencv.hpp"

typedef char byte;

/// max decoder pixel stack size
const int MaxStackSize = 4096;

class GifDecoder {
 public:
  GifDecoder();
  GifDecoder(byte* data);
  ~GifDecoder();

  void Init();

 public:
  void SetScaleWH(int sw, int sh);
  void Run();
  void Free();
  int GetStatus();
  bool ParseOk();
  int GetFrameDelay(int frameIndex);
  void GetDelays(std::vector<int> frameDelays);
  int GetFrameCount();
  IplImage* GetFirstFrame();
  int GetLoopCount();
 public:
  int width; // full image width
  int height; // full image height
 
 private:
  int status;
  bool gctFlag; // global color table used
  int gctSize;  // size of global color table
  int loopCount = 1; // iterations; 0 = repeat forever

  int* gct; // global color table
  int* lct; // local color table
  int* act; // active color table

  int bgIndex;   // background color index
  int bgColor;   // background color
  int lastBgColor; // previous bg color
  int pixelAspect; // pixel aspect ratio

  bool lctFlag; // local color table flag
  bool interlace; // interlace flag
  int lctSize; // local color table size

  int ix, iy, iw, ih; // current image rectangle
  int lrx, lry, lrw, lrh;
  
  IplImage image;       // current frame
  IplImage lastImage;   // previous frame
  GifFrame currentFrame;

  bool isShow = false;


  private byte[] block = new byte[256]; // current data block
  private int blockSize = 0; // block size

  // last graphic control extension info
  int dispose = 0;
  // 0=no action; 1=leave in place; 2=restore to bg; 3=restore to prev
  int lastDispose = 0;
  bool transparency = false; // use transparent color
  int delay = 0; // delay in milliseconds
  int transIndex; // transparent color index

  private static final int MaxStackSize = 4096;

  // LZW decoder working arrays
  private short[] prefix;
  private byte[] suffix;
  private byte[] pixelStack;
  private byte[] pixels;

  private GifFrame gifFrame; // frames read from current file
  private int frameCount;


  private byte[] gifData = null;
  private int    gifFrameIdx = 0;	

	public int scaleW = -1;
	public int scaleH = -1;

};
#endif  // JUYOU_MULTI_MEDIA_PUBLIC_GIF_DECODER_H_
