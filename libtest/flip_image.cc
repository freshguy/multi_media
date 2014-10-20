#include <cv.h>
#include <highgui.h>
#include "opencv/cv.h"
#include "opencv/highgui.h"

using namespace cv;

int main(int argc, char** argv) {
  cv::Mat src = cv::imread(argv[1]);
  cv::Mat dst;
  cv::flip(src, dst, 1);  // 1: 左右, 2: 上下
  
  imwrite("fliped_lena.jpg", dst);
  namedWindow("flipped", CV_WINDOW_AUTOSIZE);
  imshow("flipped", dst);
  waitKey(0);
  return 0;
}
