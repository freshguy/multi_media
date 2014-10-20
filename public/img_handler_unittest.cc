// Copyright (c) 2013 Juyou Inc. All rights reserved.
// Author: wangjuntao@juyou.com (Juntao Wang)

#include <stdlib.h>
#include <string>
#include <cstring>
#include "base/logging.h"
#include "base/flags.h"
#include "file/file.h"
#include "base/time.h"
#include "base/string_util.h"
#include "base/utf_string_conversions.h"
#include "base/utf_string_conversion_utils.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "juyou/third_party/CImg/CImg.h"
#include "juyou/multi_media/public/img_handler.h"
#include "juyou/multi_media/public/multimedia_transform.h"

#include <jpeglib.h>
#include <jerror.h>
#define cimg_plugin "/home/taogle/judian/news-dev/juyou/third_party/CImg/plugins/jpeg_buffer.h"

DEFINE_string(img_file, "/home/taogle/Desktop/res/good.png",
      "The load image file");

using namespace cimg_library;  // NOLINT

Magick::Image Translate(const Magick::Image& img) {
    return img;
}



namespace juyou {
namespace multimedia {

#if 0
TEST(ImageHandler, Image_test) {
  Magick::InitializeMagick("ImageHandler_unittest");

#if 0
  CImg<unsigned char> src(FLAGS_img_file.c_str());
  int width = src.width();
  int height = src.height();
  int depth = src.depth();
  int channel = src.spectrum();
  LOG(INFO) << "width: " << width
    << ", height: " << height
    << ", depth: " << depth
    << ", channel: " << channel;
  //CImgDisplay disp(src.width(), src.height());
  src.display();
#endif
 
#if 0
  CImg<unsigned char> rotate = src.rotate(0, 200, 200, 0.5, 1, 0);
  rotate.display();
#endif

  //CImg<unsigned char> addImg = src + rotate;
  //addImg.display();

#if 0
  CImg<unsigned char> crop = src.crop(100, 100, 500, 500, true);
  crop.display();
#endif

  std::string imgContent;
  file::File::ReadFileToString(FLAGS_img_file, &imgContent);
  file::File::WriteStringToFile(imgContent,
        "/home/taogle/Desktop/fileWrite.png");
#if 0
  CvMat mat = cvMat(500, 283, CV_8UC1, (void*)imgContent.c_str());
  IplImage* img = cvDecodeImage( &mat, 1 );
  if (!img) {
    LOG(ERROR) << "NULL img pointer";
    return 1;
  }

  //IplImage* img = cvCreateImageHeader(cvSize(500, 283), IPL_DEPTH_8U, 4);
  //cvSetData(img, (void*)imgContent.c_str(), 4 * 500);
  CImg<unsigned char> memImg((unsigned char*)img->imageData, 500, 283, 1, 4, false);
  memImg.display();
#endif

#if 0
  /// Cann't use
  CImg<unsigned char> img;
  img.load_jpeg_buffer(imgContent.c_str(), imgContent.size());
  img.display();
#endif

  Magick::Blob blob(reinterpret_cast<const void*>(imgContent.c_str()),
        imgContent.size());
  Magick::Image image;
  image.read(blob);
  //image.display();
  image.write("/home/taogle/Desktop/MagickXM.png");

  Magick::Image mgImg = ImgContent2Image(imgContent);
  mgImg.write("/home/taogle/Desktop/mgImg.png");

  cvNamedWindow("test", 1);
  /// Pic file content to IplImage
  int len = imgContent.size();
  std::vector<unsigned char> buf(len);  // unsigned 
  memcpy(buf.data(), imgContent.data(), len);
  cv::Mat imgMat = cv::imdecode(buf, 1);
  IplImage img = IplImage(imgMat);
  //IplImage img;
  //ImgContent2IplImage(imgContent, img);
  cvSaveImage("/home/taogle/Desktop/IplImageXM.png", &img);  //

  int width = img.width;
  int height = img.height;
  int depth = img.depth;
  int channel = img.nChannels;
  LOG(INFO) << "width: " << width << ", height: " << height
    << ", depth: " << depth << ", channel: " << channel;

  Magick::Image iplImg = CvToMagick(&img);
  iplImg.write("/home/taogle/Desktop/cv2Image.png");
  LOG(INFO) << "Storage cv2Image";

  IplImage* img2Ipl = MagickToCv(iplImg);
  cvSaveImage("/home/taogle/Desktop/Image2cv.png", img2Ipl);

  cvShowImage("test", &img);
  cvWaitKey(0);
#if 0
  CImg<unsigned char> memImg(img.imageData, width, height, depth, channel, false);
  memImg.display();
#endif
}
#endif

TEST(ImageHandler, Image_Drawable) {
#if 0
  std::string imgContent;
  file::File::ReadFileToString(FLAGS_img_file, &imgContent);
  Magick::Geometry geometry(100, 200, 0, 100);
  Magick::Blob blob(reinterpret_cast<const void*>(imgContent.c_str()),
        imgContent.size());
  Magick::Image img(blob, geometry);
  img.write("/home/taogle/Desktop/xm00.png");
  img.display();
#endif

#if 0
    // Create base image (white image of 300 by 200 pixels) 
  Magick::Image image(Magick::Geometry(300,200), Magick::Color("white"));

    // Set draw options 
    image.strokeColor("red"); // Outline color 
    image.fillColor("green"); // Fill color 
    image.strokeWidth(5);

    // Draw a circle 
    image.draw(Magick::DrawableCircle(100,100, 50,100) );

    // Draw a rectangle 
    image.draw(Magick::DrawableRectangle(200,200, 270,170) );

    // Display the result 
    image.display( ); 
#endif

#if 0
    /// Pass the test
    Magick::Color color("transparent");
    LOG(INFO) << "red: " << color.redQuantum();
    LOG(INFO) << "green: " << color.greenQuantum();
    LOG(INFO) << "blue: " << color.blueQuantum();
    LOG(INFO) << "alpha: " << color.alphaQuantum();

  Magick::Image currentImg(Magick::Geometry(480, 500), Magick::Color("transparent"));
  currentImg.write("/home/taogle/Desktop/res/current.png");

  Magick::Image img("/home/taogle/Desktop/res/good.png");
  //Magick::Image img = currentImg;
  currentImg.display();
  std::list<Magick::Drawable> drawList;
  drawList.push_back(Magick::DrawableCompositeImage(0, 100, 480, 567, img));
  currentImg.draw(drawList);
  currentImg.write("/home/taogle/Desktop/res/haha.png");
  currentImg.display();
#endif

#if 0
  /// Pass the test
  Magick::Image backG(Magick::Geometry(480, 800), Magick::Color("transparent"));
  backG.backgroundColor(Magick::Color("transparent"));
  backG.write("/home/taogle/Desktop/res/transp.png");
  backG.display();
  backG.composite(Magick::Image("/home/taogle/Desktop/res/good.png"), 0, 100,
        Magick::OverCompositeOp);  // Magick::OutCompositeOp);
  backG.write("/home/taogle/Desktop/res/composite.png");
  //backG.display();
  //backG.display();
  //backG.roll(100, 100);  // 走马灯循环
  //backG.rotate(45);
  //backG.write("/home/taogle/Desktop/res/rotate.png");
#if 0
  Magick::Image scaleImage = ScaleByCenter(&backG, 0.5);
  scaleImage.write("/home/taogle/Desktop/res/scale.png");
  scaleImage.display();
#endif
  backG.scale(Magick::Geometry(backG.columns() * 0.5, backG.rows() * 0.5));
  backG.write("/home/taogle/Desktop/res/scale.png");
#endif
  Magick::Image image("/home/taogle/Desktop/res/good.png");
  image.write("/home/taogle/Desktop/res/origImage.png");
  int orW = image.columns();
  int orH = image.rows();
  int x = 0, y = 100;
  double scale = 0.5;
  base::TimeTicks startT = base::TimeTicks::HighResNow();
  Magick::Image backG(Magick::Geometry(480, 800), Magick::Color("transparent"));
  backG.backgroundColor(Magick::Color("transparent"));
  backG.composite(image, x, y, Magick::OverCompositeOp);
  base::TimeTicks endT = base::TimeTicks::HighResNow();
  LOG(INFO) << "composite consume(ms): " << (endT - startT).InMilliseconds();
  backG.write("/home/taogle/Desktop/res/composite.png");
 
#if 0
  image.scale(Magick::Geometry(image.columns() * scale, image.rows() * scale));
  image.write("/home/taogle/Desktop/res/origScale.png");
  
  Magick::Image scaleBack(Magick::Geometry(480, 800), Magick::Color("transparent"));
  scaleBack.backgroundColor(Magick::Color("transparent"));
  int dx = (orW - scale * orW) / 2;
  int dy = (orH - scale * orH) / 2;
  scaleBack.composite(image, x + dx, y + dy, Magick::OverCompositeOp);
  scaleBack.write("/home/taogle/Desktop/res/scaleTranslate.png");
#endif
  startT = base::TimeTicks::HighResNow();
  Magick::Image scaleByCenter = ScaleByCenter(&image, scale, 480, 800,
        x, y);
  endT = base::TimeTicks::HighResNow();
  LOG(INFO) << "ScaleByCenter consume(ms): " << (endT - startT).InMilliseconds();
  scaleByCenter.write("/home/taogle/Desktop/res/scaleTranslate.png");
  
  Magick::Image imageR("/home/taogle/Desktop/res/good.png");
  double rotate = 45.0;
  base::TimeTicks startTR = base::TimeTicks::HighResNow();
  RotateAffine(&imageR, rotate);
  //imageR.draw(Magick::DrawableAngle(rotate));

  //MagickCore::Image* newImage;
  //MagickCore::ExceptionInfo exceptionInfo;
  //newImage = RotateImage(imageR.constImage(), rotate, &exceptionInfo);
  base::TimeTicks endTR = base::TimeTicks::HighResNow();
  LOG(INFO) << "rotate consume(ms): " << (endTR - startTR).InMilliseconds();
  imageR.write("/home/taogle/Desktop/res/rotateAffine.png");
#if 0
  startT = base::TimeTicks::HighResNow();
  Magick::Image rotateByCenter = RotateByCenter(&imageR, rotate, 480, 800, x, y);
  endT = base::TimeTicks::HighResNow();
  LOG(INFO) << "RotateByCenter consume(ms): " << (endT - startT).InMilliseconds();
  rotateByCenter.write("/home/taogle/Desktop/res/rotateByCenter.png");
#endif
}

TEST(CImg, Image) {
  CImg<unsigned char> src(FLAGS_img_file.c_str());
  int width = src.width();
  int height = src.height();
  int depth = src.depth();
  int channel = src.spectrum();

  base::TimeTicks startT = base::TimeTicks::HighResNow();
  CImg<unsigned char> rotate = src.rotate(45);  // 0, 200, 200, 0.5, 1, 0);
  base::TimeTicks endT = base::TimeTicks::HighResNow();
  LOG(INFO) << "CImage rotate cosume(ms): " << (endT - startT).InMilliseconds();
  //rotate.display();
}

TEST(OpencvHandleImage, allHandle) {
  IplImage* origImg = cvLoadImage("/home/taogle/Desktop/res/good.png");
  LOG(INFO) << "origImg.width = " << origImg->width
    << ", origImg.height = " << origImg->height
    << ", depth = " << origImg->depth
    << ", channels = " << origImg->nChannels;

  int scaledW = 150, scaledH = 160;
  char* rgbData = new char[scaledW * scaledH * 3];
  OpencvHandleImage(origImg->imageData, origImg->width,
        origImg->height, 0, 270, 300, 200, 90, true,
        scaledW, scaledH, rgbData);
  
  IplImage* oddImg = cvCreateImage(cvSize(scaledW, scaledH),
        IPL_DEPTH_8U, 3);
  LOG(INFO) << "After oddImg";
  oddImg->imageData = rgbData;
  cvSaveImage("/home/taogle/Desktop/odd.png", oddImg);
  delete []rgbData;
}

TEST(MagickDrawText, DrawText) {
  // Create base image (white image of 300 by 200 pixels) 
  Magick::Image image(Magick::Geometry(800, 300),
        Magick::Color("blue"));
#if 0
  // Construct drawing list 
  std::list<Magick::Drawable> drawList;
  drawList.push_back(Magick::DrawableFont("cursive", Magick::NormalStyle,
          400, Magick::ExtraExpandedStretch));
  //drawList.push_back(Magick::DrawableFont("ttf-ubuntu-font-family"));
  
  drawList.push_back(Magick::DrawableText(50, 50, "Li nan nan, I love you!", "UTF-8"));
  drawList.push_back(Magick::DrawableStrokeWidth(5)); // Stroke width 
  drawList.push_back(Magick::DrawableStrokeColor("red"));
  drawList.push_back(Magick::DrawablePointSize(40));
  // Draw everything using completed drawing list 
  image.draw(drawList);
#endif

  //image.font("helvetica");
  //image.font("chandas");
  image.font("/usr/share/fonts/truetype/newfonts/FZHCJW.TTF");
  image.fontPointsize(40);
  image.fillColor(Magick::Color("#ffffffff"));
  image.strokeWidth(3);
  image.textEncoding("UTF-8");
  image.annotate("Li nan nan, I love you! 我爱你", Magick::Geometry(50, 50, 50, 50));

  std::string str = "我是一个兵\nabc";
  LOG(INFO) << "str.length: " << str.length()
    << ", str: " << str;
  for (int i = 0; i < str.length(); i++) {
    LOG(INFO) << i << "th character: " << str[i];
  }
 
  /// convert to wstring
  std::wstring wStr = UTF8ToWide(base::StringPiece(str));
  LOG(INFO) << "wStr: " << wStr
    << ", wStr.length: " << wStr.length();
  std::string outPut;

  /// Unicode code to character
  for (int i = 0; i < wStr.length(); i++) {
    LOG(INFO) << i << "th character: " << wStr[i];
    base::WriteUnicodeCharacter(wStr[i], &outPut);
    LOG(INFO) << i << "th characterStr: " << outPut;
  }
  image.annotate(outPut, Magick::Geometry(50, 50, 50, 100));
  //image.draw(Magick::DrawableClipPath("100,100;200,100;200,200;100,200"));  // No effect
  std::vector<Point> polygonVertexs;
  polygonVertexs.push_back(Point(100, 100));
  polygonVertexs.push_back(Point(200, 100));
  polygonVertexs.push_back(Point(150, 150));
  polygonVertexs.push_back(Point(200, 200));
  polygonVertexs.push_back(Point(100, 200));
  for (int i = 0; i < image.columns(); i++) {
    for (int j = 0; j < image.rows(); j++) {
      if (!InsidePolygon(polygonVertexs, Point(i, j))) {
        image.pixelColor(i, j, "transparent"); 
      }
    }
  }
  image.backgroundColor("transparent");

  image.write("/home/taogle/Desktop/font.png");
  std::string hexStr = StringPrintf("%.x", -1);
  LOG(INFO) << "hexStr: " << hexStr;
  Magick::Color color("transparent");
  LOG(INFO) << "redQuantum: " << color.redQuantum()
    << ", greenQuantum: " << color.greenQuantum()
    << ", blueQuantum: " << color.blueQuantum()
    << ", alphaQuantum: " << color.alphaQuantum();


  std::string pointsStr = "378.0:454.0;143.0:471.0;2.0:281.0;85.0:60.0;315.0:10.0;481.0:177.0;431.0:409.0;209.0:491.0;21.0:348.0;39.0:112.0;246.0:3.0;453.0:114.0;468.0:350.0;279.0:491.0;59.0:407.0;10.0:175.0;178.0:9.0;408.0:61.0;488.0:284.0;346.0:472.0;111.0:453.0;0.0:244.0;114.0:37.0;349.0:23.0;489.0:214.0;404.0:435.0;173.0:483.0;9.0:313.0;61.0:83.0;283.0:2.0;470.0:147.0;450.0:383.0;242.0:493.0;37.0:377.0;23.0:141.0;214.0:2.0;433.0:88.0;480.0:320.0;311.0:485.0;81.0:431.0;3.0:207.0;147.0:20.0;382.0:41.0;491.0:251.0;374.0:457.0;139.0:469.0;1.0:277.0;88.0:57.0;319.0:11.0;482.0:182.0;428.0:412.0;205.0:490.0;20.0:344.0;41.0:108.0;251.0:0.0;455.0:118.0;466.0:354.0;275.0:492.0;56.0:404.0;11.0:171.0;182.0:8.0;411.0:64.0;488.0:288.0;342.0:474.0;107.0:451.0;0.0:240.0;118.0:35.0;353.0:25.0;489.0:218.0;401.0:438.0;169.0:481.0;8.0:309.0;64.0:79.0;287.0:3.0;472.0:151.0;448.0:386.0;238.0:493.0;34.0:373.0;25.0:137.0;218.0:1.0;436.0:91.0;479.0:324.0;307.0:486.0;78.0:428.0;3.0:203.0;151.0:18.0;385.0:44.0;491.0:255.0;371.0:459.0;135.0:467.0;1.0:273.0;91.0:54.0;323.0:12.0;483.0:186.0;425.0:415.0;201.0:489.0;18.0:340.0;44.0:105.0;255.0:0.0;457.0:121.0;464.0:358.0;270.0:492.0;53.0:400.0;13.0:167.0;186.0:7.0;414.0:67.0;487.0:292.0;338.0:475.0;103.0:448.0;0.0:236.0;122.0:33.0;357.0:27.0;490.0:222.0;397.0:440.0;165.0:480.0;6.0:305.0;67.0:76.0;292.0:4.0;473.0:155.0;445.0:390.0;233.0:493.0;32.0:370.0;27.0:133.0;222.0:1.0;439.0:94.0;477.0:328.0;302.0:487.0;75.0:425.0;4.0:199.0;155.0:17.0;389.0:46.0;491.0:260.0;367.0:461.0;131.0:465.0;0.0:268.0;95.0:51.0;327.0:14.0;485.0:190.0;422.0:418.0;197.0:489.0;16.0:336.0;46.0:101.0;259.0:0.0;459.0:125.0;462.0:362.0;266.0:493.0;50.0:397.0;14.0:163.0;190.0:6.0;417.0:70.0;486.0:296.0;333.0:477.0;100.0:446.0;0.0:231.0;126.0:31.0;361.0:29.0;490.0:227.0;394.0:443.0;161.0:479.0;5.0:301.0;70.0:73.0;296.0:5.0;475.0:159.0;443.0:393.0;229.0:493.0;30.0:366.0;29.0:129.0;227.0:0.0;441.0:98.0;476.0:332.0;298.0:488.0;72.0:422.0;5.0:194.0;159.0:15.0;392.0:49.0;490.0:264.0;363.0:463.0;127.0:463.0;0.0:264.0;98.0:49.0;331.0:15.0;485.0:194.0;419.0:422.0;192.0:488.0;15.0:332.0;49.0:98.0;264.0:0.0;461.0:129.0;460.0:366.0;262.0:493.0;48.0:393.0;16.0:159.0;195.0:5.0;420.0:73.0;485.0:301.0;329.0:479.0;96.0:443.0;0.0:227.0;129.0:29.0;365.0:31.0;491.0:231.0;391.0:446.0;157.0:477.0;5.0:296.0;73.0:70.0;300.0:6.0;477.0:163.0;440.0:397.0;225.0:493.0;28.0:362.0;31.0:125.0;231.0:0.0;444.0:101.0;474.0:336.0;294.0:489.0;69.0:418.0;6.0:190.0;163.0:14.0;396.0:51.0;490.0:268.0;359.0:465.0;124.0:461.0;0.0:260.0;102.0:46.0;336.0:17.0;486.0:199.0;416.0:425.0;188.0:487.0;13.0:328.0;52.0:94.0;268.0:1.0;463.0:133.0;458.0:370.0;257.0:493.0;45.0:390.0;17.0:155.0;199.0:4.0;423.0:76.0;484.0:305.0;325.0:480.0;93.0:440.0;1.0:222.0;133.0:27.0;369.0:33.0;491.0:236.0;387.0:448.0;153.0:475.0;4.0:292.0;77.0:67.0;304.0:7.0;478.0:167.0;437.0:400.0;220.0:492.0;26.0:358.0;33.0:121.0;235.0:0.0;447.0:105.0;473.0:340.0;290.0:489.0;66.0:415.0;7.0:186.0;167.0:12.0;399.0:54.0;490.0:273.0;355.0:467.0;120.0:459.0;0.0:255.0;105.0:44.0;340.0:18.0;487.0:203.0;412.0:428.0;184.0:486.0;12.0:324.0;54.0:91.0;272.0:1.0;465.0:137.0;456.0:373.0;253.0:493.0;43.0:386.0;19.0:151.0;203.0:3.0;426.0:79.0;483.0:309.0;321.0:481.0;90.0:438.0;1.0:218.0;137.0:25.0;372.0:35.0;491.0:240.0;383.0:451.0;149.0:474.0;3.0:288.0;80.0:64.0;309.0:8.0;479.0:171.0;435.0:404.0;216.0:492.0;24.0:354.0;36.0:118.0;240.0:0.0;449.0:108.0;471.0:344.0;285.0:490.0;63.0:412.0;8.0:182.0;171.0:11.0;403.0:57.0;489.0:277.0;351.0:469.0;116.0:457.0;0.0:251.0;109.0:41.0;344.0:20.0;488.0:207.0;409.0:431.0;180.0:485.0;11.0:320.0;57.0:88.0;277.0:2.0;467.0:141.0;454.0:377.0;249.0:493.0;40.0:383.0;20.0:147.0;207.0:2.0;429.0:83.0;482.0:313.0;317.0:483.0;86.0:435.0;2.0:214.0;141.0:23.0;376.0:37.0;491.0:244.0;380.0:453.0;145.0:472.0;2.0:284.0;83.0:61.0;313.0:9.0;481.0:175.0;432.0:407.0;212.0:491.0;22.0:350.0;38.0:114.0;244.0:0.0;451.0:112.0;469.0:348.0;281.0:491.0;60.0:409.0;9.0:177.0;176.0:10.0;406.0:60.0;489.0:281.0;348.0:471.0;112.0:454.0;0.0:246.0;112.0:39.0;348.0:22.0;489.0:212.0;406.0:433.0;176.0:483.0;";

  std::vector<std::string> pointsPair;
  SplitString(pointsStr, ';', &pointsPair);
  LOG(INFO) << "Point pair count: " << pointsPair.size();
  std::vector<Point> polygonPoints;
  for (int i = 0; i < pointsPair.size(); i++) {
    std::vector<std::string> points;
    SplitString(pointsPair[i], ':', &points);
    if (points.size() == 2) {
      VLOG(4) << "points[0] = " << points[0]
        << ", points[1] = " << points[1];
      double x = StringToDouble(points[0]);
      double y = StringToDouble(points[1]);
      Point point(x, y);
      VLOG(4) << "Point.x = " << point.x << ", Point.y = " << point.y;
      polygonPoints.push_back(point);
    }
  }
  LOG(INFO) << "Point count: " << polygonPoints.size();
  Magick::Image originalImg("/home/taogle/Desktop/res/polish_0.jpg");
  Magick::Image canvas(Magick::Geometry(originalImg.columns(), originalImg.rows()),
        Magick::Color("transparent"));
  canvas.backgroundColor("transparent");
  canvas.composite(originalImg, 0, 0,
        Magick::OverCompositeOp);
  ImgClipPath(polygonPoints, canvas);
  canvas.write("/home/taogle/Desktop/res/polish_0_clippath.png");

  std::string mediaInfoCommand = "ffmpeg -i /tmp/VIDEO_99d7cade1ccc4c4cab478a5d1b33d4b420140104224348.mp4 2>&1|grep -r rotate|awk -F\" \" '{print $NF}'";
  // std::string mediaInfoCommand = "ffmpeg -i /tmp/message_video_561b24da27fb42b5bc543f8318b24f27.mp4 2>&1|grep -r rotate|awk -F\" \" '{print $NF}'";
  LOG(INFO) << "mediaInfoCommand: " << mediaInfoCommand;
  char result[100];
  if (0 == mysystem(mediaInfoCommand.c_str(), result, 100)) {
    LOG(INFO) << "Get Video MediaInfo successfully";
    LOG(INFO) << "result: " << result;
  } else {
    LOG(ERROR) << "Get Video MediaInfo failed";
    LOG(INFO) << "result: " << result;
  }
  std::string resultStr = result;
  int resultInt = StringToInt(std::string(result));
  LOG(INFO) << "rotateResult: " << resultInt;
}
}  // namespace multimedia
}  // namespace juyou
