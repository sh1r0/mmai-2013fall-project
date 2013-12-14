.PHONY: all run clean

CC=g++
PROG=grabcut
CFLAGS=
OPENCV_VER=246
OPENCV_INC="C:\opencv\build\include" 
OPENCV_LIB="C:\opencv\build\x86\mingw\lib" 
OPENCV=-I$(OPENCV_INC) -L$(OPENCV_LIB) -lopencv_core$(OPENCV_VER) -lopencv_highgui$(OPENCV_VER) -lopencv_imgproc$(OPENCV_VER)

all: $(PROG)

$(PROG): $(PROG).cpp
	$(CC) -o $@ $^ $(CFLAGS) $(OPENCV)
