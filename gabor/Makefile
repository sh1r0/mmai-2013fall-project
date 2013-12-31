.PHONY: all clean

# opencv
OPENCV_INC="C:\opencv-2.4.7.2\include"
OPENCV_LIB="C:\opencv-2.4.7.2\x64\mingw\lib"
OPENCV_VER=247
OPENCV_FLAGS=-I$(OPENCV_INC) -L$(OPENCV_LIB) -lopencv_core$(OPENCV_VER) -lopencv_highgui$(OPENCV_VER) -lopencv_imgproc$(OPENCV_VER)
# boost
BOOST_INC="D:\MinGW\include"
BOOST_LIB="D:\MinGW\lib"
BOOST_FLAGS=-I$(BOOST_INC) -L$(BOOST_LIB) -lboost_program_options
# fftw
FFTW_LIB="C:\fftw-3.3.3-dll64"
FFTW_FLAGS=-I$(FFTW_LIB) -L$(FFTW_LIB) -lfftw3f

CC=g++
CFLAGS=$(OPENCV_FLAGS) $(BOOST_FLAGS) $(FFTW_FLAGS) -lm -O3

all:
	$(CC) gaborUtil.cpp $(CFLAGS) -c
	$(CC) gabor.cpp $(CFLAGS) -c
	$(CC) LogGabor.cpp $(CFLAGS) -c
	$(CC) gaborUtil.o gabor.o LogGabor.o Gabor_main.cpp $(CFLAGS) -o gabor
	rm -f *.o

clean:
	rm gabor
