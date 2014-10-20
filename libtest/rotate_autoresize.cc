#include <opencv/cv.h>
#include <opencv/highgui.h>

/// http://blog.csdn.net/xiaowei_cqu/article/details/7616044

//旋转图像内容不变，尺寸相应变大
IplImage* rotateImage1(IplImage* img,int degree){
	double angle = degree  * CV_PI / 180.; // 弧度  
	double a = sin(angle), b = cos(angle); 
	int width = img->width;  
	int height = img->height;  
	int width_rotate = int(height * fabs(a) + width * fabs(b));  
	int height_rotate = int(width * fabs(a) + height * fabs(b));  
	//旋转数组map
	// [ m0  m1  m2 ] ===>  [ A11  A12   b1 ]
	// [ m3  m4  m5 ] ===>  [ A21  A22   b2 ]
	float map[6];
	CvMat map_matrix = cvMat(2, 3, CV_32F, map);  
	// 旋转中心
	CvPoint2D32f center = cvPoint2D32f(width / 2, height / 2);  
	cv2DRotationMatrix(center, degree, 1.0, &map_matrix);  
	map[2] += (width_rotate - width) / 2;  
	map[5] += (height_rotate - height) / 2;  
	IplImage* img_rotate = cvCreateImage(cvSize(width_rotate, height_rotate), 8, 3); 
	//对图像做仿射变换
	//CV_WARP_FILL_OUTLIERS - 填充所有输出图像的象素。
	//如果部分象素落在输入图像的边界外，那么它们的值设定为 fillval.
	//CV_WARP_INVERSE_MAP - 指定 map_matrix 是输出图像到输入图像的反变换，
	cvWarpAffine( img,img_rotate, &map_matrix, CV_INTER_LINEAR | CV_WARP_FILL_OUTLIERS, cvScalarAll(0));  
	return img_rotate;
}

//旋转图像内容不变，尺寸相应变大
IplImage* rotateImage2(IplImage* img, int degree)  
{  
	double angle = degree  * CV_PI / 180.; 
	double a = sin(angle), b = cos(angle); 
	int width=img->width, height=img->height;
	//旋转后的新图尺寸
	int width_rotate= int(height * fabs(a) + width * fabs(b));  
	int height_rotate=int(width * fabs(a) + height * fabs(b));  
	IplImage* img_rotate = cvCreateImage(cvSize(width_rotate, height_rotate), img->depth, img->nChannels);  
	cvZero(img_rotate);  
	//保证原图可以任意角度旋转的最小尺寸
	int tempLength = sqrt((double)width * width + (double)height *height) + 10;  
	int tempX = (tempLength + 1) / 2 - width / 2;  
	int tempY = (tempLength + 1) / 2 - height / 2;  
	IplImage* temp = cvCreateImage(cvSize(tempLength, tempLength), img->depth, img->nChannels);  
	cvZero(temp);  
	//将原图复制到临时图像tmp中心
	cvSetImageROI(temp, cvRect(tempX, tempY, width, height));  
	cvCopy(img, temp, NULL);  
	cvResetImageROI(temp);  
	//旋转数组map
	// [ m0  m1  m2 ] ===>  [ A11  A12   b1 ]
	// [ m3  m4  m5 ] ===>  [ A21  A22   b2 ]
	float m[6];  
	int w = temp->width;  
	int h = temp->height;  
	m[0] = b;  
	m[1] = a;  
	m[3] = -m[1];  
	m[4] = m[0];  
	// 将旋转中心移至图像中间  
	m[2] = w * 0.5f;  
	m[5] = h * 0.5f;  
	CvMat M = cvMat(2, 3, CV_32F, m);  
	cvGetQuadrangleSubPix(temp, img_rotate, &M);  
	cvReleaseImage(&temp);  
	return img_rotate;
}

int main(int argc, char** argv) {
  IplImage* src = cvLoadImage(argv[1]);
  IplImage* dst = rotateImage2(src, 45);
  cvSaveImage("/home/taogle/Desktop/test_lena.jpg", dst);
  return 0;
}
