#include <opencv/cv.h>
#include <opencv/highgui.h>

IplImage *rotateImage(const IplImage *src, int angleDegrees)
{    
    IplImage *imageRotated = cvCloneImage(src);

    if(angleDegrees!=0){
        CvMat* rot_mat = cvCreateMat(2,3,CV_32FC1);
    
        // Compute rotation matrix
        CvPoint2D32f center = cvPoint2D32f( cvGetSize(imageRotated).width/2, cvGetSize(imageRotated).height/2 );
        cv2DRotationMatrix( center, angleDegrees, 1, rot_mat );

        // Do the transformation
        cvWarpAffine( src, imageRotated, rot_mat );
    }

    return imageRotated;
}


int main(int argc, char** argv) {
          
          IplImage* img;
          IplImage* rotated_img;

          int angle=0;
          
          //creating the window with a track bar
          cvNamedWindow("MyWindow");
          cvCreateTrackbar("Angle", "MyWindow", &angle, 360, 0);
          
          while(true){
              //load the original image
              img = cvLoadImage(argv[1]);
              
              //rotate the image
              rotated_img=rotateImage(img,angle);
              cvSaveImage("/home/taogle/Desktop/test_lena.jpg", rotated_img);

              //display the rotated image
              cvShowImage("MyWindow", rotated_img); 

              //clean up
              cvReleaseImage(&img);
              cvReleaseImage(&rotated_img);
              
              //if user press 'ESC' button, program quit the while loop
              int c=cvWaitKey(50);               
              if(c==27) break;
          }
          
          
          cvDestroyWindow("MyWindow");
          
          return 0;
}
