CPP = g++
SRC = src/process.cpp src/file_utils.cpp src/image_process.cpp

TESTSRC = src/test/image_process_test.cpp src/image_process.cpp

CPPFLAGS =-I include -O3 -std=c++11 -g -msse2
LIBS = -lrt -lm -lOpenCL

all: bin bin/process

bin :
	mkdir -p bin

bin/process:	src/process.cpp
	$(CPP) $(CPPFLAGS) -o $@ $(SRC) $(LIBS)

bin/ip_test: src/test/image_process_test.cpp
	$(CPP) $(CPPFLAGS) -o $@ $(TESTSRC) $(LIBS)

clean:
	rm -f bin/process

.PHONY: all clean
