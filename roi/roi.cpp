/***************objectmarker.cpp******************

Object marker for creating a list of ROIs to use for (e.g.) OpenCV cascade training. 

Requires OpenCV, and various common C/C++ libraries. 

author: bdh@machinehum.com
originally based on objectmarker.cpp by: achu_wilson@rediffmail.com
*/

#include <opencv/cv.h>
#include <opencv/cvaux.h>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <stdio.h>
#include <sys/uio.h>
#include <string>
#include <fstream>
#include <sstream>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

using namespace cv;
using namespace std;

Mat image;
Mat image2;
Mat image3;
int max_height = 800;
float scale_factor = 1;
int roi_x0=0;
int roi_y0=0;
int roi_x1=0;
int roi_y1=0;
int numOfRec=0;
int startDraw = 0;
int aspect_x = 1;
int aspect_y = 1;
int mod_x = 0;
int mod_y = 0;
string window_name="";

void on_mouse(int event,int x,int y,int flag, void *param)
{
    if ( event == CV_EVENT_LBUTTONDOWN )
    {
        if ( !startDraw )
        {
          roi_x0=x;
          roi_y0=y;
          startDraw = 1;
        } else {
          roi_x1=x;
          roi_y1=y;

          // enforce ROI aspect ratio
          if (
               ( (roi_x0 > x) && (roi_y0 > y) )
               ||
               ( (roi_x0 < x) && (roi_y0 < y) )
             )
            roi_x1 = roi_x0 + ( aspect_x * ( roi_y1 - roi_y0 ) / aspect_y );
          else
            roi_x1 = roi_x0 + ( aspect_x * ( roi_y0 - roi_y1 ) / aspect_y );

          // redraw ROI selection in green when finished
          image2 = image3.clone();
          rectangle( image2, cvPoint( roi_x0, roi_y0 ), cvPoint( roi_x1, roi_y1 ), CV_RGB(50,255,50), 1 );
          imshow( window_name, image2 );

          startDraw = 0;
        }
    }
    if ( event == CV_EVENT_MOUSEMOVE && startDraw )
    {
        mod_x = x; 
        mod_y = y;

        // enforce ROI aspect ratio
        if (  
             ( (roi_x0 > x) && (roi_y0 > y) )
             ||
             ( (roi_x0 < x) && (roi_y0 < y) )
           )
          mod_x = roi_x0 + ( aspect_x * ( mod_y - roi_y0 ) / aspect_y );
        else
          mod_x = roi_x0 + ( aspect_x * ( roi_y0 - mod_y ) / aspect_y );

        //redraw ROI selection
        image2 = image3.clone();
        rectangle(image2,cvPoint( roi_x0, roi_y0 ),cvPoint( mod_x, mod_y ),CV_RGB(255,0,100),1);
        imshow(window_name,image2);
    }

}

int main(int argc, char** argv)
{
  char iKey=0;
  string strPrefix;
  string strPostfix;
  string fullPath;
  string destPath;

  string input_directory = ".";
  string pos_directory = "marked";
  string neg_directory = "negative";
  string positive_file = "pos.txt";
  string negative_file = "neg.txt";

  struct stat filestat;
  int c;

  while (optind < argc) {
    if ((c = getopt(argc, argv, "m:u:p:n:h:x:y:")) != -1) 
      switch (c)
        {
        case 'm':
          pos_directory = optarg;
          break;
        case 'u':
          neg_directory = optarg;
          break;
        case 'p':
          positive_file = optarg;
          break;
        case 'n':
          negative_file = optarg;
          break;
        case 'h':
          max_height = std::stoi(optarg);
          break;
        case 'x':
          aspect_x = std::stoi(optarg);
          break;
        case 'y':
          aspect_y = std::stoi(optarg);
          break;
        case '?':
          if (optopt == 'd' || optopt == 'p' || optopt == 'n')
            fprintf (stderr, "Option -%c requires an argument.\n", optopt);
          else if (isprint (optopt))
            fprintf (stderr, "Unknown option `-%c'.\n", optopt);
          else
            fprintf (stderr,
                     "Unknown option character `\\x%x'.\n",
                     optopt);
          return 1;
        default:
          abort ();
        }
    else {
      input_directory = argv[optind];
      optind++;
    }    
  }

  pos_directory = input_directory + "/" + pos_directory;
  neg_directory = input_directory + "/" + neg_directory;
  positive_file = pos_directory + "/" + positive_file;
  negative_file = neg_directory + "/" + negative_file;

  /* Create output directories if necessary */
  DIR *dir_d = opendir( pos_directory.c_str());
  if ( dir_d == NULL ) {
    mkdir( pos_directory.c_str(), S_IRWXU | S_IRWXG );
  } else {
    closedir(dir_d);
  }

  DIR *dir_e = opendir( neg_directory.c_str());
  if ( dir_e == NULL ) {
    mkdir( neg_directory.c_str(), S_IRWXU | S_IRWXG );
  } else {
    closedir(dir_e);
  }

  // Open input directory 
  DIR *dir_p = opendir( input_directory.c_str() );
  struct dirent *dir_entry_p;

  if(dir_p == NULL) {
    fprintf(stderr, "Failed to open directory %s\n", input_directory.c_str());
    return -1;
  }

  // GUI 
  window_name = "ROI Marking";
  namedWindow(window_name,1);
  setMouseCallback(window_name,on_mouse, NULL);

  // init output streams
  ofstream positives(positive_file.c_str(), std::ofstream::out | std::ofstream::app);
  ofstream negatives(negative_file.c_str(), std::ofstream::out | std::ofstream::app);

  // Iterate over directory 
  while((dir_entry_p = readdir(dir_p)) != NULL)
  {
    // Skip directories
    string relPath = input_directory + "/" + dir_entry_p->d_name;
    if ( stat( relPath.c_str(), &filestat ) == -1 ) continue;
    if (S_ISDIR( filestat.st_mode )) continue;

    strPostfix = "";
    numOfRec = 0;
    scale_factor = 1;

    if(strcmp(dir_entry_p->d_name, ""))
      fprintf(stderr, "Examining file %s\n", dir_entry_p->d_name);

    strPrefix = dir_entry_p->d_name;
    fullPath  = input_directory + "/" + strPrefix;
    printf("Loading image %s\n", strPrefix.c_str());

    image = imread(fullPath.c_str(),1);

    if(! image.data )                              // Check for invalid input
    {
        cout <<  "Could not open or find the image" << std::endl ;
        continue;
    }

    do
    {
      // Use scratch version of image for display, scaling down if necessary.
      Size sz = image.size();
      if ( sz.height > max_height ) {
        if (sz.width > sz.height) {
          scale_factor = (float)max_height / (float)sz.width;
        } else {
          scale_factor = (float)max_height / (float)sz.height;
        }
        resize(image, image3, Size(), scale_factor, scale_factor, INTER_AREA);
      } else {
        image3 = image.clone();
      }
      imshow(window_name,image3);

      // Start taking input
      iKey=cvWaitKey(0);
      switch(iKey)
      {
        case 27:
          image.release();
          image2.release();
          image3.release();
          destroyWindow(window_name);
          closedir(dir_p);
          return 0;
        case 32:
          numOfRec++;
#ifdef DEBUG
          printf("   %d. rect x=%d\ty=%d\tx2h=%d\ty2=%d\n",numOfRec,roi_x0,roi_y0,roi_x1,roi_y1);
#endif
	  strPostfix += " " + to_string( int( min(roi_x0,roi_x1) / scale_factor ) ) + " " + 
                        to_string( int( min(roi_y0,roi_y1) / scale_factor ) ) + " " +
                        to_string( int( ( abs(roi_x1 - roi_x0) ) / scale_factor ) ) + " " + 
                        to_string( int( ( abs(roi_y1 - roi_y0) ) / scale_factor ) );
        break;
      }
    }
    while(iKey!=97);

    if (iKey==97) {
      if (numOfRec > 0) {
        positives << strPrefix << " "<< numOfRec << strPostfix <<"\n";
        destPath = pos_directory + "/" + strPrefix;  
      } else {
        negatives << strPrefix << "\n";
        destPath = neg_directory + "/" + strPrefix;
      }
    }
    rename( fullPath.c_str(), destPath.c_str() );
  }

  image.release();
  image2.release();
  image3.release();
  destroyWindow(window_name);
  closedir(dir_p);
  return 0;
}
