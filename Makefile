CPP = g++
SRC = src/process.cpp src/cl_process.cpp  src/file_utils.cpp  src/image_process.cpp

CFLAGS =-I include -O3 -std=c++11 -g
LIBS = -lrt -lm -lOpenCL

all: bin bin/process

bin :
	mkdir -p bin

bin/process:	src/process.cpp
	$(CPP) $(CFLAGS) -o $@ $(SRC) $(LIBS)

clean:
	rm -f bin/process

.PHONY: all clean
