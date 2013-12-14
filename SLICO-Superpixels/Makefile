.PHONY: all

CC=g++
PROG=main
CFLAGS=
OPENCV_VER=246
OPENCV_INC="C:\opencv\build\include"
OPENCV_LIB="C:\opencv\build\x86\mingw\lib"
OPENCV=-I$(OPENCV_INC) -L$(OPENCV_LIB) -lopencv_core$(OPENCV_VER) -lopencv_highgui$(OPENCV_VER)

all: $(PROG)

$(PROG): SLIC.cpp $(PROG).cpp
	$(CC) -o $@ $^ $(CFLAGS) $(OPENCV)
