CPP = g++
SRC = src/process.cpp src/file_utils.cpp src/image_process.cpp
OLDSRC = oldsrc/process.cpp oldsrc/file_utils.cpp oldsrc/image_process.cpp
TESTSRC = src/test/image_process_test.cpp src/image_process.cpp

CPPFLAGS =-I include -O3 -std=c++11 -msse
LIBS = 

all: bin bin/process bin/oldprocess bin/ip_test

bin :
	mkdir -p bin

bin/process:	src/process.cpp
	$(CPP) $(CPPFLAGS) -o $@ $(SRC) $(LIBS)

bin/oldprocess: oldsrc/process.cpp
	$(CPP) $(CPPFLAGS) -o $@ $(OLDSRC) $(LIBS)

bin/ip_test: src/test/image_process_test.cpp
	$(CPP) $(CPPFLAGS) -o $@ $(TESTSRC) $(LIBS)

clean:
	rm -f bin/process

.PHONY: all clean
