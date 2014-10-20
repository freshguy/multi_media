#include<cv.h>
#include<highgui.h>


IplImage* inpaint_mask = 0;
IplImage* img0 = 0, *img = 0;
CvPoint prev_pt = {-1,-1};

void on_mouse( int event, int x, int y, int flags, void* param)
{
    if( !img )
        return;
    if( event == CV_EVENT_LBUTTONUP || !(flags & CV_EVENT_FLAG_LBUTTON) )
        prev_pt = cvPoint(-1,-1);
    else if( event == CV_EVENT_LBUTTONDOWN )
        prev_pt = cvPoint(x,y);
    else if( event == CV_EVENT_MOUSEMOVE && (flags & CV_EVENT_FLAG_LBUTTON) )
    {
        CvPoint pt = cvPoint(x,y);
        if( prev_pt.x < 0 ) {
          printf("LINE < 0: event = %d, x = %d, y = %d, flags = %d\n", event, x, y, flags);
            prev_pt = pt;
        }
        printf("LINE: event = %d, x = %d, y = %d, flags = %d\n", event, x, y, flags);
        cvLine( inpaint_mask, prev_pt, pt, cvScalarAll(0), 2, 8, 0 ); // 模板上画
        cvLine( img, prev_pt, pt, cvScalarAll(255), 2, 8, 0 ); // 原图上画
        prev_pt = pt;
        cvShowImage( "image", img );
    }
    if(event == CV_EVENT_LBUTTONUP)
    {
      printf("BUTTONUP\n");
        cvFloodFill(inpaint_mask, cvPoint(x, y), cvScalarAll(0)); // 填充抠图模板
        cvShowImage( "watershed transform", inpaint_mask ); // 显示模板
        cvSaveImage("maskImg.bmp", img);  // 保存在原图上画线后的图，如结果中第一个图
        IplImage *segImage=cvCreateImage(cvGetSize(img),8,3);
        //cvZero(segImage); // 加这句可使结果第三个图的背景为黑色
        cvCopy(img,segImage, inpaint_mask);  // 使用模板拷贝
        cvShowImage( "image", segImage ); // 显示抠图结果
        cvSaveImage("segImage.bmp",segImage); // 保存抠图结果
        cvReleaseImage(&segImage);
    }
}

int main(int argc, char** argv)
{
    char* filename0 = (char*)"lena.jpg";
    char* filename = argc >= 2 ? argv[1] : filename0;
    if( (img0 = cvLoadImage(filename,-1)) == 0 )
        return 0;
    cvNamedWindow( "image", 0 );
    cvNamedWindow("watershed transform", 0);
    img = cvCloneImage( img0 );
    inpaint_mask = cvCreateImage( cvGetSize(img), 8, 1 );
    cvSet( inpaint_mask, cvScalarAll(255)); // 255 将模板设为白色
    cvShowImage( "image", img ); // 显示原图
    cvShowImage( "watershed transform", inpaint_mask ); // 显示模板原图
    cvSetMouseCallback( "image", on_mouse, 0 ); 
    cvWaitKey(0);
    cvDestroyWindow("image");
    cvDestroyWindow("watershed transform");
    cvReleaseImage(&img);
    cvReleaseImage(&img0);
    cvReleaseImage(&inpaint_mask);
    return 0;
}


#if 0
#include "cv.h"
#include "highgui.h"
#include <stdio.h>
#include <stdlib.h>

IplImage* inpaint_mask = 0;
IplImage* img0 = 0, *img = 0, *inpainted = 0;
CvPoint prev_pt = {-1,-1};

void on_mouse( int event, int x, int y, int flags, void* zhang)
{
  if( !img )
   return;
  if( event == CV_EVENT_LBUTTONUP || !(flags & CV_EVENT_FLAG_LBUTTON) )
    prev_pt = cvPoint(-1,-1);//初始化
  else if( event == CV_EVENT_LBUTTONDOWN )
    prev_pt = cvPoint(x,y);
  else if( event == CV_EVENT_MOUSEMOVE && (flags & CV_EVENT_FLAG_LBUTTON) )
  {//手一直在绘画
    CvPoint pt = cvPoint(x,y);
    if( prev_pt.x < 0 )
    prev_pt = pt;
    cvLine( inpaint_mask, prev_pt, pt, cvScalarAll(255), 2, 8, 0 );
    cvLine( img, prev_pt, pt, cvScalarAll(255), 2, 8, 0 );
    prev_pt = pt;
    cvShowImage( "image", img );
  }
   if(event == CV_EVENT_LBUTTONUP)
    {
        cvFloodFill(inpaint_mask,cvPoint(x,y),cvScalarAll(255));
        cvSaveImage("maskImg.bmp",inpaint_mask);
        IplImage *segImage=cvCreateImage(cvGetSize(img),8,3);;
        cvCopy(img,segImage,inpaint_mask);
        cvSaveImage("segImage.bmp",segImage);
        // cvCopy(segImage,proc->img);
        cvReleaseImage(&segImage);
        cvDestroyWindow("Key Image");
    }
}

int main( int argc, char** argv )
{
   char* filename = argc >= 2 ? argv[1] : (char*)"lena.jpg";
   if( (img0 = cvLoadImage(filename,-1)) == 0 )
     return 0;
   printf( "Hot keys: \n");
   cvNamedWindow( "image", 1 );
   img = cvCloneImage( img0 );
   inpainted = cvCloneImage( img0 );
   inpaint_mask = cvCreateImage( cvGetSize(img), 8, 1 );
    cvSet( inpaint_mask, cvScalarAll(255)); // 255 将模板设为白色
   cvZero( inpaint_mask );
   cvZero( inpainted );
   cvShowImage( "image", img );
   cvShowImage( "watershed transform", inpainted );
   cvSetMouseCallback( "image", on_mouse, 0 );   
   cvWaitKey(0);
   return 0;
}
#endif
