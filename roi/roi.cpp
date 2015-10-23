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

Mat image;  // original image
Mat image2; // scaled image, where confirmed ROIs are built up for display
Mat image3; // scratch copy of image2 to display current drawing on top of other ROIs.

std::vector<Rect> rois;

int max_height = 800;
float scale_factor = 1;

int aspect_x = 1;
int aspect_y = 1;

int startDraw = 0;
int roi_x0=0;
int roi_y0=0;
int mod_x = 0;
int mod_y = 0;

string windowName="";

void on_mouse(int event,int x,int y,int flag, void *param)
{
  // enforce ROI aspect ratio
  if ( startDraw ) {
    mod_y = y;
    if (
         ( (roi_x0 > x) && (roi_y0 > y) )
         ||
         ( (roi_x0 < x) && (roi_y0 < y) )
       )
      mod_x = roi_x0 + ( aspect_x * ( y - roi_y0 ) / aspect_y );
    else
      mod_x = roi_x0 + ( aspect_x * ( roi_y0 - y ) / aspect_y );
  }

  if ( event == CV_EVENT_LBUTTONDOWN )
  {
    if ( !startDraw )
    {
      roi_x0=x;
      roi_y0=y;
      startDraw = 1;
    } else {
      // redraw putative ROI selection in yellow when finished. will be redrawn in green when confirmed with <space>
      rectangle( image3, cvPoint( roi_x0, roi_y0 ), cvPoint( mod_x, mod_y ), CV_RGB(255,255,50), 1 );
      imshow( windowName, image3 );
      startDraw = 0;
    }
  }
  if ( event == CV_EVENT_MOUSEMOVE && startDraw )
  {
    //redraw ROI selection
    image3 = image2.clone();
    rectangle(image3,cvPoint( roi_x0, roi_y0 ),cvPoint( mod_x, mod_y ),CV_RGB(255,5,50),1);
    imshow(windowName,image3);
  }
  if ( event == EVENT_LBUTTONDBLCLK )
  {
    roi_x0 = 0;
    roi_y0 = 0;
    mod_x = 0;
    mod_y = 0;
    startDraw = 0;
  }

}

int main(int argc, char** argv)
{
  char iKey=0;
  string fileForOutput;

  string fullPath;
  string destPath;

  string inputDirectory = ".";
  string outputDirectory = ".";
  string positiveDirectory = "positive";
  string negativeDirectory = "negative";
  string positiveFile = "pos.txt";
  string negativeFile = "neg.txt";

  struct stat filestat;
  int c;

  while (optind < argc) {
    if ((c = getopt(argc, argv, "o:m:u:p:n:h:x:y:")) != -1) 
      switch (c)
        {
        case 'o': 
          outputDirectory = optarg;
          break;
        case 'm':
          positiveDirectory = optarg;
          break;
        case 'u':
          negativeDirectory = optarg;
          break;
        case 'p':
          positiveFile = optarg;
          break;
        case 'n':
          negativeFile = optarg;
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
      inputDirectory = argv[optind];
      optind++;
    }
  }

  positiveDirectory = outputDirectory + "/" + positiveDirectory;
  negativeDirectory = outputDirectory + "/" + negativeDirectory;
  positiveFile = positiveDirectory + "/" + positiveFile;
  negativeFile = negativeDirectory + "/" + negativeFile;

  /* Create output directories if necessary */
  DIR *dir_d = opendir( positiveDirectory.c_str());
  if ( dir_d == NULL ) {
    mkdir( positiveDirectory.c_str(), S_IRWXU | S_IRWXG );
  } else {
    closedir(dir_d);
  }

  DIR *dir_e = opendir( negativeDirectory.c_str());
  if ( dir_e == NULL ) {
    mkdir( negativeDirectory.c_str(), S_IRWXU | S_IRWXG );
  } else {
    closedir(dir_e);
  }

  // Open input directory 
  DIR *dir_p = opendir( inputDirectory.c_str() );
  struct dirent *dir_entry_p;

  if(dir_p == NULL) {
    fprintf(stderr, "Failed to open directory %s\n", inputDirectory.c_str());
    return -1;
  }

  // GUI 
  windowName = "ROI Marking";
  namedWindow(windowName,1);
  setMouseCallback(windowName,on_mouse,NULL);

  // init output streams
  ofstream positives(positiveFile.c_str(), std::ofstream::out | std::ofstream::app);
  ofstream negatives(negativeFile.c_str(), std::ofstream::out | std::ofstream::app);

  // Iterate over directory 
  while((dir_entry_p = readdir(dir_p)) != NULL)
  {
    // Skip directories
    string relPath = inputDirectory + "/" + dir_entry_p->d_name;
    if ( stat( relPath.c_str(), &filestat ) == -1 ) continue;
    if (S_ISDIR( filestat.st_mode )) continue;

    // Initialize
    rois.clear();
    roi_x0 = 0;
    roi_y0 = 0;
    startDraw = 0;
    scale_factor = 1;


    if(strcmp(dir_entry_p->d_name, ""))
      fprintf(stderr, "Examining file %s\n", dir_entry_p->d_name);

    fileForOutput = dir_entry_p->d_name;
    fullPath  = inputDirectory + "/" + fileForOutput;
    printf("Loading image %s\n", fileForOutput.c_str());

    image = imread(fullPath.c_str(),1);

    if(! image.data )                              // Check for invalid input
    {
        cout <<  "Could not open or find the image" << std::endl ;
        continue;
    }

    // Use scratch version of image for display, scaling down if necessary.
    Size sz = image.size();
    if ( sz.height > max_height ) {
      scale_factor = (float)max_height / (float)sz.height;
      resize(image, image2, Size(), scale_factor, scale_factor, INTER_AREA);
    } else {
      image2 = image.clone();
    }
    imshow(windowName,image2);

    // Input loop
    do
    {
      iKey=cvWaitKey(0);
      switch(iKey)
      {
        case 27:
          image.release();
          image2.release();
          image3.release();
          destroyWindow(windowName);
          closedir(dir_p);
          return 0;
        case 32: // <space> confirms ROI
          if ( (startDraw == 0) && (abs( roi_x0 - mod_x ) > 0) ) 
          {
            int origin_x = min(roi_x0, mod_x);
            int origin_y = min(roi_y0, mod_y);
            int width = abs(mod_x - roi_x0);
            int height = abs(mod_y - roi_y0);
            int terminus_x = origin_x + width;
            int terminus_y = origin_y + height;

            // Scale up to map to original size
            rois.push_back( Rect( int( origin_x / scale_factor ),
                                  int( origin_y / scale_factor ),
                                  int( width / scale_factor ),
                                  int( height / scale_factor )
                                )
                          );

            // Re-draw in green 
            rectangle( image2, cvPoint( origin_x,origin_y ), cvPoint( terminus_x,terminus_y ), CV_RGB(50,255,50), 1 );
            imshow(windowName,image2);
          }
        default: 
          imshow(windowName,image2);
        break;
      }
    }
    while( iKey != 97 );

    if ( iKey == 97 ) {
      if ( rois.size() > 0 ) {
        positives << fileForOutput << " " << to_string(rois.size());
        for(std::vector<Rect>::iterator r = rois.begin(); r != rois.end(); ++r) {
          positives << " " << to_string(r->x) << " " << to_string(r->y) << " " << to_string(r->width) << " " << to_string(r->height);
        }
        positives << "\n";
        destPath = positiveDirectory + "/" + fileForOutput;  
      } else {
        negatives << fileForOutput << "\n";
        destPath = negativeDirectory + "/" + fileForOutput;
      }
    }
    rename( fullPath.c_str(), destPath.c_str() );
  }

  image.release();
  image2.release();
  image3.release();
  destroyWindow(windowName);
  closedir(dir_p);
  return 0;
}
