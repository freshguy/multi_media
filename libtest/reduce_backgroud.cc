#include <cv.h>
#include <highgui.h>
#include <math.h>
//#include <stdlib.h>
//#include <stdio.h>
 
 
int main(int argc, char* argv[])
{
    IplImage *src = 0; //定义源图像指针
 
    IplImage *tmp = 0; //定义临时图像指针
 
    IplImage *src_back = 0; //定义源图像背景指针
 
    IplImage *dst_gray = 0; //定义源文件去掉背景后的目标灰度图像指针
 
    IplImage *dst_bw = 0; //定义源文件去掉背景后的目标二值图像指针
 
    IplImage *dst_contours = 0; //定义轮廓图像指针
 
    IplConvKernel *element = 0; //定义形态学结构指针
 
    int Number_Object =0; //定义目标对象数量
 
    int contour_area_tmp = 0; //定义目标对象面积临时寄存器
 
    int contour_area_sum = 0; //定义目标所有对象面积的和
 
    int contour_area_ave = 0; //定义目标对象面积平均值
 
    int contour_area_max = 0; //定义目标对象面积最大值
 
    CvMemStorage *stor = 0;
    CvSeq * cont = 0;
    //CvContourScanner contour_scanner; 
    //CvSeq * a_contour= 0;
 
    //1.读取和显示图像
 
    /* the first command line parameter must be image file name */
    if ( argc == 2 && (src = cvLoadImage(argv[1], 0))!=0 )
    {
        ;
    }
    else
    {
        src = cvLoadImage("rice.jpg", 0);
    }
    cvNamedWindow( "src", CV_WINDOW_AUTOSIZE );
    cvShowImage( "src", src );
    //cvSmooth(src, src, CV_MEDIAN, 3, 0, 0, 0); //中值滤波，消除小的噪声；
 
    //2.估计图像背景
 
    tmp = cvCreateImage( cvGetSize(src), src->depth, src->nChannels);
    src_back = cvCreateImage( cvGetSize(src), src->depth, src->nChannels);
    //创建结构元素
 
    element = cvCreateStructuringElementEx( 4, 4, 1, 1, CV_SHAPE_ELLIPSE, 0);
    //用该结构对源图象进行数学形态学的开操作后，估计背景亮度
 
    cvErode( src, tmp, element, 10);
    cvDilate( tmp, src_back, element, 10);
    cvNamedWindow( "src_back", CV_WINDOW_AUTOSIZE );
    cvShowImage( "src_back", src_back );
 
    //3.从源图象中减区背景图像
 
    dst_gray = cvCreateImage( cvGetSize(src), src->depth, src->nChannels);
    cvSub( src, src_back, dst_gray, 0);
    cvNamedWindow( "dst_gray", CV_WINDOW_AUTOSIZE );
    cvShowImage( "dst_gray", dst_gray );
 
    //4.使用阈值操作将图像转换为二值图像
 
    dst_bw = cvCreateImage( cvGetSize(src), src->depth, src->nChannels);
    cvThreshold( dst_gray, dst_bw ,50, 255, CV_THRESH_BINARY ); //取阈值为50把图像转为二值图像
 
    //cvAdaptiveThreshold( dst_gray, dst_bw, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, 3, 5 );
 
    cvNamedWindow( "dst_bw", CV_WINDOW_AUTOSIZE );
    cvShowImage( "dst_bw", dst_bw );
 
      //5.检查图像中的目标对象数量
 
    stor = cvCreateMemStorage(0);
      cont = cvCreateSeq(CV_SEQ_ELTYPE_POINT, sizeof(CvSeq), sizeof(CvPoint), stor);
      Number_Object = cvFindContours(dst_bw, stor, &cont, sizeof(CvContour), 
      CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0) ); //找到所有轮廓
 
    printf("Number_Object: %d\n", Number_Object);
 
    //6.计算图像中对象的统计属性
 
    dst_contours = cvCreateImage( cvGetSize(src), src->depth, src->nChannels);
    cvThreshold( dst_contours, dst_contours ,0, 255, CV_THRESH_BINARY_INV); //在画轮廓前先把图像变成白色
 
    for(;cont;cont = cont->h_next)
    {
        cvDrawContours( dst_contours, cont, CV_RGB(255, 0, 0), CV_RGB(255, 0, 0), 0, 1, 8, cvPoint(0, 0) ); //绘制当前轮廓
 
        contour_area_tmp = fabs(cvContourArea( cont, CV_WHOLE_SEQ )); //获取当前轮廓面积
 
        if( contour_area_tmp > contour_area_max )
        {
            contour_area_max = contour_area_tmp; //找到面积最大的轮廓
 
        }
        contour_area_sum += contour_area_tmp; //求所有轮廓的面积和
 
    }
    contour_area_ave = contour_area_sum/ Number_Object; //求出所有轮廓的平均值
 
    printf("contour_area_ave: %d\n", contour_area_ave );
    printf("contour_area_max: %d\n", contour_area_max );
    cvNamedWindow( "dst_contours", CV_WINDOW_AUTOSIZE );
    cvShowImage( "dst_contours", dst_contours );
 
    cvWaitKey(-1); //等待退出
 
    cvReleaseImage(&src);
    cvReleaseImage(&tmp);
    cvReleaseImage(&src_back);
    cvReleaseImage(&dst_gray);
    cvReleaseImage(&dst_bw);
    cvReleaseImage(&dst_contours);
    cvReleaseMemStorage(&stor);
    cvDestroyWindow( "src" );
    cvDestroyWindow( "src_back" );
    cvDestroyWindow( "dst_gray" );
    cvDestroyWindow( "dst_bw" );
    cvDestroyWindow( "dst_contours" );
    //void cvDestroyAllWindows(void);
 
    return 0;
}
