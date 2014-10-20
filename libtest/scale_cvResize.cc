#include <highgui.h>
#include <math.h>
#include "opencv2/imgproc/types_c.h"
#include "opencv2/imgproc/imgproc_c.h"

int main(int argc, char* argv[]) {
  IplImage *src = 0;          //源图像指针
  IplImage *dst = 0;          //目标图像指针
  float scale = 0.618;        //缩放倍数为0.618倍
  CvSize dst_cvsize;          //目标图像尺寸

  /* the first command line parameter must be image file name */
  if ( argc == 2 && (src = cvLoadImage(argv[1], -1))!=0 )
  {
    //如果命令行传递了需要打开的图片就无须退出，所以注释掉下面一行！
    //return -1;
  } else {
    src = cvLoadImage("tiger.jpg");    //载入工作目录下文件名为“tiger.jpg”的图片。
  }

  dst_cvsize.width = src->width *  scale;      //目标图像的宽为源图象宽的scale倍
  dst_cvsize.height = src->height * scale;    //目标图像的高为源图象高的scale倍

  dst = cvCreateImage( dst_cvsize, src->depth, src->nChannels);   //构造目标图象
  cvResize(src, dst, CV_INTER_LINEAR);    //缩放源图像到目标图像

  cvNamedWindow( "src",   CV_WINDOW_AUTOSIZE );   //创建用于显示源图像的窗口
  cvNamedWindow( "dst",   CV_WINDOW_AUTOSIZE );   //创建用于显示目标图像的窗口

  cvShowImage( "src", src );      //显示源图像
  cvShowImage( "dst", dst );      //显示目标图像

  cvWaitKey(-1);      //等待用户响应

  cvReleaseImage(&src);   //释放源图像占用的内存
  cvReleaseImage(&dst);   //释放目标图像占用的内存
  cvDestroyWindow( "src" );   //销毁窗口“src”
  cvDestroyWindow( "dst" );   //销毁窗口“dst”
  //void cvDestroyAllWindows(void);

  return 0;
}
