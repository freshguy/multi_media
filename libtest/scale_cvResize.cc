#include <highgui.h>
#include <math.h>
#include "opencv2/imgproc/types_c.h"
#include "opencv2/imgproc/imgproc_c.h"

int main(int argc, char* argv[]) {
  IplImage *src = 0;          //Դͼ��ָ��
  IplImage *dst = 0;          //Ŀ��ͼ��ָ��
  float scale = 0.618;        //���ű���Ϊ0.618��
  CvSize dst_cvsize;          //Ŀ��ͼ��ߴ�

  /* the first command line parameter must be image file name */
  if ( argc == 2 && (src = cvLoadImage(argv[1], -1))!=0 )
  {
    //��������д�������Ҫ�򿪵�ͼƬ�������˳�������ע�͵�����һ�У�
    //return -1;
  } else {
    src = cvLoadImage("tiger.jpg");    //���빤��Ŀ¼���ļ���Ϊ��tiger.jpg����ͼƬ��
  }

  dst_cvsize.width = src->width *  scale;      //Ŀ��ͼ��Ŀ�ΪԴͼ����scale��
  dst_cvsize.height = src->height * scale;    //Ŀ��ͼ��ĸ�ΪԴͼ��ߵ�scale��

  dst = cvCreateImage( dst_cvsize, src->depth, src->nChannels);   //����Ŀ��ͼ��
  cvResize(src, dst, CV_INTER_LINEAR);    //����Դͼ��Ŀ��ͼ��

  cvNamedWindow( "src",   CV_WINDOW_AUTOSIZE );   //����������ʾԴͼ��Ĵ���
  cvNamedWindow( "dst",   CV_WINDOW_AUTOSIZE );   //����������ʾĿ��ͼ��Ĵ���

  cvShowImage( "src", src );      //��ʾԴͼ��
  cvShowImage( "dst", dst );      //��ʾĿ��ͼ��

  cvWaitKey(-1);      //�ȴ��û���Ӧ

  cvReleaseImage(&src);   //�ͷ�Դͼ��ռ�õ��ڴ�
  cvReleaseImage(&dst);   //�ͷ�Ŀ��ͼ��ռ�õ��ڴ�
  cvDestroyWindow( "src" );   //���ٴ��ڡ�src��
  cvDestroyWindow( "dst" );   //���ٴ��ڡ�dst��
  //void cvDestroyAllWindows(void);

  return 0;
}
