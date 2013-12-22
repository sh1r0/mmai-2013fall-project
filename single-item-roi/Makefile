.PHONY: all run clean

CC=g++
PROG=extract_roi
CFLAGS=
OPENCV_VER=247
OPENCV_INC="C:\opencv-2.4.7.2\include"
OPENCV_LIB="C:\opencv-2.4.7.2\x64\mingw\lib"
OPENCV=-I$(OPENCV_INC) -L$(OPENCV_LIB) -lopencv_core$(OPENCV_VER) -lopencv_highgui$(OPENCV_VER) -lopencv_imgproc$(OPENCV_VER)

all: $(PROG)

$(PROG): $(PROG).cpp
	$(CC) -o $@ $^ $(CFLAGS) $(OPENCV)
