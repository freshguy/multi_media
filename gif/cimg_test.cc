// Copyright (c) 2013 Juyou Inc. All rights reserved.
// Author: wangjuntao@juyou.com (Juntao Wang)

#include "base/logging.h"
#include "base/flags.h"
#include "util/global_init/global_init.h"
#include "juyou/third_party/CImg/CImg.h"

DEFINE_string(img_file, "/home/taogle/Desktop/gif/0.png",
      "The load image file");

using namespace cimg_library;  // NOLINT

#if 0
int main(int argc, char* argv[]) {
    CImg<unsigned char> src(FLAGS_img_file.c_str());
    // 设置原图大小，貌似haar计算要求图像宽高是4的倍数
    //src.resize( src.width()-src.width()%4, src.height()-src.height()%4);
    CImgList<unsigned char> visu;
        
    visu.push_back(src.get_crop(0,0,src.width()/2,src.height()/2));
    LOG(INFO) << "here";
        visu.push_back(src.get_quantize(5));
        visu.push_back(src.get_rotate(45,1));
        visu.push_back(src.get_permute_axes("yxzv"));
        visu.push_back(src.get_erode(5));
        visu.push_back(src.get_haar());
        visu.push_back(src.get_dilate(3));
        visu.push_back(src.get_blur(3));
        visu.push_back(src.get_noise(3));
        visu.push_back(src.get_deriche(3));
        visu.push_back(src.get_blur_anisotropic(8));
        // visu.push_back(src.get_blur_bilateral(1,2));
        visu.push_back(src.get_blur_patch(4,3));
        visu.push_back(src.get_sharpen(3));
        visu.push_back(src.get_blur_median(3));
        //; //如果愿意可以测试更多CImg的图像处理方法
   
    LOG(INFO) << "after construct src";
    // 用来显示效果
    CImgDisplay disp(src.width()*2, src.height());
    int i=0;
    unsigned char textcolor[] = { 255,255,255 };
    while(!disp.is_closed() && !disp.is_key() && !disp.is_keyESC())
    {
        i = i % visu.size();
        char buf[20];
        ::sprintf(buf,"img:%d",i);
        //显示效果，（CImg << CImg）会生成一个新的CImgList
        //左边是原图，右边是处理图，外加写了个序号在上面以便区别
        disp.display( src << (+visu[i]).draw_text(0,0,buf,textcolor) ).wait();
        //按方向键下则显示下一个
        if(disp.is_keyARROWDOWN()) i++;
        //方向键上则显示上一个
        if(disp.is_keyARROWUP())
        {
            i--;
            if(i<0) {
              i=visu.size()-1;
            }
        }
    }
    return 0;
}
#endif

int main(int argc, char* argv[]) {
  util::GlobalInit global_init(&argc, &argv);
  CImg<unsigned char> src(FLAGS_img_file.c_str());
  LOG(INFO) << "width: " << src.width()
    << ", height: " << src.height();
  //CImgDisplay disp(src.width(), src.height());
  src.display();
  CImg<unsigned char> rotate = src.rotate(90, 200, 200, 0.5, 1, 1);
  rotate.display();
  CImg<unsigned char> crop = src.crop(100, 100, 700, 100, 100, 500, 700, 500, true);
  crop.display();
  return 0;
}
