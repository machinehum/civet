#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <vector>

using namespace cv;
using namespace std;

int main (int argc, char** argv) {
  char iKey=0;
  Mat img = imread(argv[1]);
  CascadeClassifier cascade;
  if (cascade.load(argv[2])) {
    vector<Rect> faces;
    cascade.detectMultiScale(img, faces, 1.1, 3, CV_HAAR_FIND_BIGGEST_OBJECT, Size(24, 24));

    printf("{\n");
    printf("\"file\": %s,\n", argv[1]); 
    printf("\"count\": %zd", faces.size());
    
    for (int i = 0; i < faces.size(); i++) {
      Rect r = faces[i];
      if ( i == 0 ) {
        printf(",\n");
      }
      printf("\"object\": {\n");
      printf("\t\"x\": %d,\n", r.x);
      printf("\t\"y\": %d,\n", r.y);
      printf("\t\"width\": %d,\n", r.width);
      printf("\t\"height\": %d\n", r.height);
      if (i < (faces.size() - 1)) {
        printf("},\n");
      } else {
        printf("}\n");
      }
    }
 
    printf("\n}\n");
  
    if ( argv[3] && !strcmp(argv[3], "-i") ) {
      for (int i = 0; i < faces.size(); i++) {
        Rect r = faces[i]; 
        rectangle(img, r, Scalar(0,255,0), 2);
      }
      imshow( argv[1], img );
      iKey=waitKey(0);
      destroyWindow(argv[1]);
    }

  }
  img.release();
  return 0;
}
