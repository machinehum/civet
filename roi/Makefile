CC = g++
CFLAGS =
LDFLAGS = -I/usr/local/include/opencv -lopencv_highgui -lopencv_core -lopencv_video -lopencv_imgproc -lopencv_imgcodecs
ALL = roi

all: $(ALL)

roi: roi.o
	$(CC) -o $@ $^ $(LDFLAGS)

roi.o: roi.cpp
	$(CC) -c roi.cpp

.PHONY: clean

install:
	cp roi /usr/local/bin/

clean:
	rm -rf *.o core* $(ALL)

