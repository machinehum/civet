CC = g++
CFLAGS =
LDFLAGS = -I/usr/local/include/opencv2 -L/usr/local/lib -lopencv_core -lopencv_imgcodecs -lopencv_imgproc -lopencv_calib3d -lopencv_video -lopencv_features2d -lopencv_ml -lopencv_highgui -lopencv_objdetect 
ALL = detect-multiscale

all: $(ALL)

detect-multiscale: detect-multiscale.o
	$(CC) $(LDFLAGS) -o $@ $^

detect-multiscale.o: detect-multiscale.cpp
	$(CC) -c detect-multiscale.cpp

.PHONY: clean

install:
	sudo cp detect-multiscale /usr/local/bin/
clean:
	rm -rf *.o core* $(ALL)
