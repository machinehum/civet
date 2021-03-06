#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>

#include <iostream>
#include <vector>

#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

using namespace cv;
using namespace std;

char iKey = 0;

int detect( string imagePath, string cascadePath, vector<Rect> & rois, int minNbr, bool equalize, bool greyscale, int aspect_x, int aspect_y, bool interactive, string windowName) {
  Mat img = imread(imagePath.c_str(),1);
  Size sz = img.size();
  if ( sz.height < 1 || sz.width < 1 ) {
    return -1;
  }
  CascadeClassifier cascade;

  if (cascade.load(cascadePath)) {
    //vector<Rect> rois;
    //std::vector<Rect> v;
    Mat frame_gray;
    cvtColor( img, frame_gray, COLOR_BGR2GRAY );
    equalizeHist( frame_gray, frame_gray );
    cascade.detectMultiScale(frame_gray, rois, 1.2, minNbr, CV_HAAR_FIND_BIGGEST_OBJECT, Size(aspect_x, aspect_y), Size((aspect_x*70),(aspect_y*70)));  
    //rois = &v; 
    if ( interactive ) {
      for (int i = 0; i < rois.size(); i++) {
        Rect r = rois[i]; 
        rectangle(img, r, Scalar(0,255,0), 2);
      }
      imshow( windowName, img );
      iKey=waitKey(0);
        if (iKey == 27 ) {
          img.release(); 
          exit(1);
        }
    }

  }
  img.release();
  return 0;
}

void printJSON(string imagePath, vector<Rect> & rois){
  printf("{\n");
  printf("\"path\": \"%s\",\n", imagePath.c_str()); 
  printf("\"count\": %zd", rois.size());
  if ( rois.size() > 0 ) {
    printf(",\n\"objects\": [\n");
    for (int i = 0; i < rois.size(); i++) {
      Rect r = rois[i];
      printf("\t{\"x\": %d, \"y\": %d, \"width\": %d, \"height\": %d }", r.x, r.y, r.width, r.height);
      if (i < (rois.size() - 1)) {
        printf(",\n");
      }
    }
    printf("\n\t]");
  }
  printf("\n}\n");
}

int main (int argc, char** argv) {

  int c;
  string optString = "c:n:x:y:ieg";
  string needsArgs = "cxyn";
  string cascadeXML = "";
  string inputNode = "";
  int minNbr = 3;
  int aspect_x = 24;
  int aspect_y = 24;
  bool interactive = false;
  bool equalize = false;
  bool greyscale = false;
  struct stat nodeStat;
  struct dirent *dir_entry_p;
  string imagePath = "";
  string windowName = "Detect Multiscale";
  bool firstFile = true;
  std::vector<Rect> rois;

  while ( optind < argc) {
    if ((c = getopt(argc, argv, optString.c_str())) != -1 ) {
      switch (c)
        {
        case 'c': 
          cascadeXML = optarg;
          break;
        case 'i':
          interactive = true;
          break;
        case 'e':
          equalize = true;
          break;
        case 'g':
          greyscale = true;
          break;
        case 'x':
          aspect_x = std::stoi(optarg);
          break;
        case 'y':
          aspect_y = std::stoi(optarg);
          break;
        case 'n':
          minNbr = std::stoi(optarg);
          break;  
        case '?':
          if (needsArgs.find(optopt) != std::string::npos)
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
    } else {
      inputNode = argv[optind];
      optind++;
    }
  }

  // Hit our input node and see what we're dealing with
  if( stat(inputNode.c_str(),&nodeStat) == 0 ) {
    if( nodeStat.st_mode & S_IFDIR ) { // Directory
      DIR *dir_p = opendir( inputNode.c_str() );
      printf("{\n\"files\": [\n" );
      while((dir_entry_p = readdir(dir_p)) != NULL) {
        // Skip directories
        string relPath = inputNode + "/" + dir_entry_p->d_name;
        if ( stat( relPath.c_str(), &nodeStat ) == -1 ) continue;
        if (S_ISDIR( nodeStat.st_mode )) continue;
        if(!strcmp(dir_entry_p->d_name, "")) continue;
        if ( detect(relPath, cascadeXML, rois, minNbr, equalize, greyscale, aspect_x, aspect_y, interactive, windowName) != -1 ) {
          if (!firstFile)
            printf(",\n");
          printJSON(relPath, rois);
          firstFile = false;
        }
      }
      printf("]\n}\n");
    } else if( nodeStat.st_mode & S_IFREG ) { // Single file
      detect(inputNode, cascadeXML, rois, minNbr, equalize, greyscale, aspect_x, aspect_y, interactive, windowName);
    } else {
      fprintf(stderr, "%s is neither a file nor directory\n", inputNode.c_str());
    }
  } else {
      fprintf(stderr, "Could not open %s\n", inputNode.c_str());
  }

  if (interactive)
    destroyWindow(windowName);

  return 0;
}


