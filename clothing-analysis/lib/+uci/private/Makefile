.PHONY: all

MATLABROOT=c:/Progra~1/MATLAB/R2012b
CC=g++
CFLAG= -Wall -m64 -O3 -I$(MATLABROOT)/extern/include $(SRC) $(LIBS) -o $(EXE)
MEXFLAG=-m64 -shared -DMATLAB_MEX_FILE -I$(MATLABROOT)/extern/include -Wl,--export-all-symbols $(LIBS)
LIBS= -L$(MATLABROOT)/bin/win64 -L$(MATLABROOT)/extern/lib/win64/microsoft -lmex -lmx -lmwlapack -lmwblas -leng
EXE=
MEXTGT=dt
SRC=
MEXSRC=

all: $(MEXTGT)

$(EXE): $(SRC)
	$(CC) $(CFLAG) -ladvapi32 -luser32 -lgdi32 -lkernel32 -lmingwex -o $(EXE)
	@rm -f *.o*

$(MEXTGT): $(MEXTGT).cc
	$(CC) $(MEXFLAG) $^ -o $@.mexw64 -ladvapi32 -luser32 -lgdi32 -lkernel32 -lmingwex
	@rm -f *.o*
