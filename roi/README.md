# The ROI-dentifier 0.1

*author*: Brian Hayden <bdh@machinehum.com>
originally based on objectmarker.cpp by: achu_wilson@rediffmail.com

*Copyright 2015*, distributed under a *BSD license* without warranty. 

# Summary 
The object-marker iterates through a set of images and allows the user to mark 
"Regions of Interest" (ROIs) for use downstream by OpenCV (etc.) training software. 

# Requirements
1. *nix, including Linux or OS X. There is no attempt at Windows compatibility at this time.
2. OpenCV 3.
3. C++, a sane C++0x compiler, and the standard libraries. 

*Note:* This program is built for OpenCV 3, with the C++ API. Trying to mix it in with older versions,
or the C API, is very likely to cause you pain. 

# Build 
1. Modify the Makefile as necessary for your environment. You'll need to supply the location
   of your OpenCV headers (default is /usr/local/include/opencv), as well as the libraries if
   they are somewhere other than /usr/local/lib. 
2. make (and make install, if desired).

# Usage

`object-marker <input-directory> [params]`

`<input-directory>`
	The directory that object-marker will scan for images to be marked.
	Defaults to the current working directory.

`-o <output-directory>`
  The directory where output will be written. 
  Defaults to the current working directory.

`-m <marked-directory>`
	The directory to which images will be moved if ROIs have been marked
 	on them ("Positive" images) *relative to* `<input-directory>`.
	Defaults to <output-directory>/marked

`-u <unmarked-directory>`
  The directory to which images will be moved if ROIs have NOT been marked
  on them ("Negative" images) *relative to* `<input-directory>`.
  Defaults to <output-directory>/negative

`-p <positive-file>`
	The file to which positive file paths, and ROI data, will be written
  *relative to* `<marked-directory>`. 
	Defaults to <marked-directory>/pos.txt.

`-n <negative-file>`
	The file to which negative file paths will be written
  *relative to* `<unmarked-directory>`. 
	Defaults to <unmarked-directory>/neg.txt.

`-h <max-height>`
	The maximum height for the image display window. You want to use this if your 
	display is smaller than your images to be marked -- OpenCV's display built-ins
	aren't smart about scaling, or scrolling.

`-x <aspect-ratio-width> -y <aspect-ratio-height>`
	The default aspect ratio for the ROI selection tool is 1:1 (square), since this
	is the default input ratio for programs like opencv_createsamples. If you will
	be using a different aspect ratio downstream (such as the 1:2 common with HoG
	training for figures), supply that here. 

# Operation 
- Click the mouse to set a starting corner for the ROI. 
- As you move the mouse, you'll see a red rectangle locked to the current aspect ratio. 
- Click a second time to define the opposite corner of the ROI. 
- The rectangle will now turn green to indicate that it is set, and ready for you to 
  confirm. 
- To confirm and record this ROI, press `<space bar>`.
- When you are done selecting ROIs on the image, press *a* to move to the next image. 
  This also triggers the ROIs, if any, to be written to file, and moves the image to
  either the "marked" or "negative" directory (depending on whether any ROIs were
  marked). 
- Continue until all images are marked, or press `<escape>` to quit. Because images are
  moved out of the input directory once they are dealt with, and the positive/negative
  output files are appended to rather than overwritten, you may quit at any time and 
  pick up where you left off by simply launching object-marker with the same parameters.

Have fun!
