#include <unistd.h>
#include <algorithm>
#include <cassert>
#include <stdexcept>
#include <vector>
#include <cstdio>
#include <iostream>
#include <string>
#include <sys/time.h>
#include <time.h>
#include <stddef.h>
#include <sys/sysinfo.h>

#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

/*
This is a program for performing morphological operations in gray-scale
images, and in particular very large images. The general idea of
morphological operations can be found here:
http://homepages.inf.ed.ac.uk/rbf/HIPR2/matmorph.htm

Functionality:

The program performs either open or close operations,
where an open is an erode followed by a dilate, and
a close is a dilate followed by an open.

Our erode operation replaces each pixel with the minimum
of its Von Neumann neighbourhood, i.e. the left-right
up-down cross we have used before. Dilate does the same, but
takes the maximum. At each boundary, the neigbourhood
it truncated to exclude parts which extend beyond the
image. So for example, in the top-left corner the
neighbourhood consists of just {middle,down,right}.

Images are input and output as raw binary gray-scale
images, with no header. The processing parameters are
set on the command line as "width height [bits] [levels]":
	- Width: positive integer, up to and including 2^24
	- Height: positive integer, up to and including 2^24
	- Bits: a binary power <=32, default=8
	- Levels: number of times to erode before dilating (or vice-versa),
	          default=1

A constraint is that mod(Width*bits,64)==0. There
is no constraint on image size, beyond that imposed
by the width, height, and bits parameters. To be
more specific: you may be asked to process images up
to 2^44 or so in pixel count, and your program won't
be running on  a machine with 2^41 bytes of host
memory, let alone GPU memory.

Image format:

Images are represented as packed binary, in little-endian
scanline order. For images with less than 8 bits per pixel, the
left-most pixel is in the MSB. For example, the 64x3 1-bit
image with packed hex representation:

    00 00 00 00 ff ff ff ff  00 00 f0 f0 00 00 0f 0f  01 02 04 08 10 20 40 80

represents the image:

    0000000000000000000000000000000011111111111111111111111111111111
	0000000000000000111100001111000000000000000000000000111100001111
	0000000100000010000001000000100000010000001000000100000010000000

You can use imagemagick to convert to and from binary representation.
For example, to convert a 512x512 image input.png to 2-bit:

	convert input.png -depth 2 gray:input.raw

and to convert output.raw back again:

	convert -size 512x512 -depth 2 gray:output.raw output.png
	
They can also read/write on stdin/stdout for streaming. But, it is
also easy to generate images programmatically, particularly when
dealing with large images. You can even use /dev/zero and
friends if you just need something large to shove through,
or want to focus on performance.

Correctness:

The output images should be bit-accurate correct. Any conflict
between the operation of this program and the definition of the
image processing and image format should be seen as a bug in
this program. For example, this program cannot handle very
large images, which is a bug.

Performance and constraints:

The metric used to evaluate this work is maximum pixel
latency. Pixel latency is defined as the time between
a given pixel entering the program, and the transformed
pixel at the same co-ordinates leaving the program. The
goal is to minimise the maximimum latency over all
pixels. Latency measuring does not start until the first
pixel enters the pipeline, so performance measurement
is "on-hold" till that point. However, your program
must eventually read the first pixel...

The program should support the full spectrum of input
parameters correctly, including all image sizes and
all pixel depths. However, particular emphasis is
placed on very large 1-bit and 8-bit images. The
images you'll process can have any pixel distribution, but
they are often quite sparse, meaning a large proportion
of the pixels are zero. The zero to non-zero ratio may
rise to 1:1000000 for very large images, with a large
amount of spatial clustering of the non-zero pixels.
That may be useful in some cases... or not.

Execution and compilation:

Your program will be executed on a number of machines,
and it is your job to try to make it work efficiently
on whatever it finds. It should work correctly
on whatever hardware it is executed on, whether it has a K40
or a dual-core Celeron. Performance is subordinate to
correctness, so be careful when allocating buffers etc.
It is always better to fall back to software if
something goes wrong with OpenCL setup, rather than crashing
or having the program quit.

Both OpenCL 1.1 and TBB 4.2 will be available at compilation
time, and it is up to you what you want to use. Recall
the "on-hold" metric, and consider the possibilities...

Submission:

Your submission should consist of a tar.gz (not a rar or a zip),
and there should be a directory called "src", which contains
all the .cpp files that need to be compiled together to
create your executable. Any header files need to be in that
directory, or included with relative paths.

The compiler will make OpenCL 1.1 available for #include from
"CL/*" and "OpenCL/*", and TBB 4.2 from "tbb/*". No other
libraries should be assumed, beyond standard C++11 libraries.

Your program will always be run with the "src" directory
as its working directory, so you can load any kernel
files from there, or from sub-directories of "src".

As well as your source code, you should also include a
"readme.txt" or "readme.pdf", which covers the following:
- What is the approach used to improve performance, in
  terms of algorithms, patterns, and optimisations.
- A description of any testing methodology or verification.
- A summary of how work was partitioned within the pair,
  including planning, design, and testing, as well as coding.
This document does not need to be long, it is not a project
report.

As well as the code and the document, feel free to include
any other interesting artefacts, such as testing code
or performance measurements. Just make sure to clean out
anything large, like test images, object files, executables.

Marking scheme:

33% : Performance, relative to other implementations
33% : Correctness
33% : Code review (style, appropriateness of approach, readability...)

Plagiarism:

Everyones submission will be different for this coursework,
there is little chance of seeing the same code by accident.
However, you can use whatever code you like from this file.
You may also use third-party code, but you need to attribute
it very clearly and carefully, and be able to justify why
it was used - submissions created from mostly third-party
code will be frowned upon, but only in terms of marks.

Any code transfer between pairs will be treated as plagiarism.
I would suggest you keep your code private, not least because
you are competing with the others.

*/


////////////////////////////////////////////
// Routines for bringing in binary images

/*! Reverse the orders of bits if necessary
	\note This is laborious and a bit pointless. I'm sure it could be removed, or at least moved...
*/
uint64_t shuffle64(unsigned bits, uint64_t x)
{
	if(bits==1){
		x=((x&0x0101010101010101ull)<<7)
			| ((x&0x0202020202020202ull)<<5)
			| ((x&0x0404040404040404ull)<<3)
			| ((x&0x0808080808080808ull)<<1)
			| ((x&0x1010101010101010ull)>>1)
			| ((x&0x2020202020202020ull)>>3)
			| ((x&0x4040404040404040ull)>>5)
			| ((x&0x8080808080808080ull)>>7);
	}else if(bits==2){
		x=((x&0x0303030303030303ull)<<6)
			| ((x&0x0c0c0c0c0c0c0c0cull)<<2)
			| ((x&0x3030303030303030ull)>>2)
			| ((x&0xc0c0c0c0c0c0c0c0ull)>>6);
	}else if(bits==4){
		x=((x&0x0f0f0f0f0f0f0f0full)<<4)
			| ((x&0xf0f0f0f0f0f0f0f0ull)>>4);
	}
	return x;
}

/*! Take data packed into incoming format, and exand to one integer per pixel */
void unpack_blob(unsigned w, unsigned h, unsigned bits, const uint64_t *pRaw, uint32_t *pUnpacked)
{
	uint64_t buffer=0;
	unsigned bufferedBits=0;
	
	const uint64_t MASK=0xFFFFFFFFFFFFFFFFULL>>(64-bits);
	
	for(unsigned i=0;i<w*h;i++){
		if(bufferedBits==0){
			buffer=shuffle64(bits, *pRaw++);
			bufferedBits=64;
		}
		
		pUnpacked[i]=buffer&MASK;
		buffer=buffer>>bits;
		bufferedBits-=bits;
	}
	
	assert(bufferedBits==0);
}

/*! Go back from one integer per pixel to packed format for output. */
void pack_blob(unsigned w, unsigned h, unsigned bits, const uint32_t *pUnpacked, uint64_t *pRaw)
{
	uint64_t buffer=0;
	unsigned bufferedBits=0;
	
	const uint64_t MASK=0xFFFFFFFFFFFFFFFFULL>>(64-bits);
	
	for(unsigned i=0;i<w*h;i++){
		buffer=buffer | (uint64_t(pUnpacked[i]&MASK)<< bufferedBits);
		bufferedBits+=bits;
		
		if(bufferedBits==64){
			*pRaw++ = shuffle64(bits, buffer);
			buffer=0;
			bufferedBits=0;
		}
	}
	
	assert(bufferedBits==0);
}

bool read_blob(int fd, uint64_t cbBlob, void *pBlob)
{
	uint8_t *pBytes=(uint8_t*)pBlob;
	
	uint64_t done=0;
	while(done<cbBlob){
		int todo=(int)std::min(uint64_t(1)<<30, cbBlob-done);		
		
		int got=read(fd, pBytes+done, todo);
		if(got==0 && done==0)
			return false;	// end of file
		if(got<=0)
			throw std::invalid_argument("Read failure.");
		done+=got;
	}
	
	return true;
}

void write_blob(int fd, uint64_t cbBlob, const void *pBlob)
{
	const uint8_t *pBytes=(const uint8_t*)pBlob;
	
	uint64_t done=0;
	while(done<cbBlob){
		int todo=(int)std::min(uint64_t(1)<<30, cbBlob-done);
		
		int got=write(fd, pBytes+done, todo);
		if(got<=0)
			throw std::invalid_argument("Write failure.");
		done+=got;
	}
}

///////////////////////////////////////////////////////////////////
// Basic image processing primitives

uint32_t vmin(uint32_t a, uint32_t b)
{ return std::min(a,b); }

uint32_t vmin(uint32_t a, uint32_t b, uint32_t c)
{ return std::min(a,std::min(b,c)); }

uint32_t vmin(uint32_t a, uint32_t b, uint32_t c, uint32_t d)
{ return std::min(std::min(a,d),std::min(b,c)); }

uint32_t vmin(uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t e)
{ return std::min(e, std::min(std::min(a,d),std::min(b,c))); }


void erode(unsigned w, unsigned h, const std::vector<uint32_t> &input, std::vector<uint32_t> &output)
{
	auto in=[&](int x, int y) -> uint32_t { return input[y*w+x]; };
	auto out=[&](int x, int y) -> uint32_t & {return output[y*w+x]; };
	
	for(unsigned x=0;x<w;x++){
		if(x==0)
		{
			out(0,0)=vmin(in(0,0), in(0,1), in(1,0));
			for(unsigned y=1;y<h-1;y++)
			{
				out(0,y)=vmin(in(0,y), in(0,y-1), in(1,y), in(0,y+1));
			}
			out(0,h-1)=vmin(in(0,h-1), in(0,h-2), in(1,h-1));
		}
		else if(x<w-1)
		{
			out(x,0)=vmin(in(x,0), in(x-1,0), in(x,1), in(x+1,0));
			for(unsigned y=1;y<h-1;y++)
			{
				out(x,y)=vmin(in(x,y), in(x-1,y), in(x,y-1), in(x,y+1), in(x+1,y));
			}
			out(x,h-1)=vmin(in(x,h-1), in(x-1,h-1), in(x,h-2), in(x+1,h-1));
		}
		else
		{
			out(w-1,0)=vmin(in(w-1,0), in(w-1,1), in(w-2,0));
			for(unsigned y=1;y<h-1;y++)
			{
				out(w-1,y)=vmin(in(w-1,y), in(w-1,y-1), in(w-2,y), in(w-1,y+1));
			}
			out(w-1,h-1)=vmin(in(w-1,h-1), in(w-1,h-2), in(w-2,h-1));
		}
	}
}

uint32_t vmax(uint32_t a, uint32_t b)
{ return std::max(a,b); }

uint32_t vmax(uint32_t a, uint32_t b, uint32_t c)
{ return std::max(a,std::max(b,c)); }

uint32_t vmax(uint32_t a, uint32_t b, uint32_t c, uint32_t d)
{ return std::max(std::max(a,d),std::max(b,c)); }

uint32_t vmax(uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t e)
{ return std::max(e, std::max(std::max(a,d),std::max(b,c))); }

void dilate(unsigned w, unsigned h, const std::vector<uint32_t> &input, std::vector<uint32_t> &output)
{
	auto in=[&](int x, int y) -> uint32_t { return input[y*w+x]; };
	auto out=[&](int x, int y) -> uint32_t & {return output[y*w+x]; };
	
	for(unsigned x=0;x<w;x++)
	{
		if(x==0)
		{
			out(0,0)=vmax(in(0,0), in(0,1), in(1,0));
			for(unsigned y=1;y<h-1;y++)
			{
				out(0,y)=vmax(in(0,y), in(0,y-1), in(1,y), in(0,y+1));
			}
			out(0,h-1)=vmax(in(0,h-1), in(0,h-2), in(1,h-1));
		}
		else if(x<w-1)
		{
			out(x,0)=vmax(in(x,0), in(x-1,0), in(x,1), in(x+1,0));
			for(unsigned y=1;y<h-1;y++)
			{
				out(x,y)=vmax(in(x,y), in(x-1,y), in(x,y-1), in(x,y+1), in(x+1,y));
			}
			out(x,h-1)=vmax(in(x,h-1), in(x-1,h-1), in(x,h-2), in(x+1,h-1));
		}
		else
		{
			out(w-1,0)=vmax(in(w-1,0), in(w-1,1), in(w-2,0));
			for(unsigned y=1;y<h-1;y++){
				out(w-1,y)=vmax(in(w-1,y), in(w-1,y-1), in(w-2,y), in(w-1,y+1));
			}
			out(w-1,h-1)=vmax(in(w-1,h-1), in(w-1,h-2), in(w-2,h-1));
		}
	}
}

///////////////////////////////////////////////////////////////////
// Composite image processing

void process(int levels, unsigned w, unsigned h, unsigned /*bits*/, std::vector<uint32_t> &pixels)
{
	std::vector<uint32_t> buffer(w*h);
	
	// Depending on whether levels is positive or negative,
	// we flip the order round.
	auto fwd=levels < 0 ? erode : dilate;
	auto rev=levels < 0 ? dilate : erode;
	
	for(int i=0;i<std::abs(levels);i++)
	{
		fwd(w, h, pixels, buffer);
		std::swap(pixels, buffer);
	}
	for(int i=0;i<std::abs(levels);i++)
	{
		rev(w,h,pixels, buffer);
		std::swap(pixels, buffer);
	}
}

// You may want to play with this to check you understand what is going on
void invert(int levels, unsigned w, unsigned h, unsigned bits, std::vector<uint32_t> &pixels)
{
	uint32_t mask=0xFFFFFFFFul>>bits;
	
	for(unsigned i=0;i<w*h;i++)
	{
		pixels[i]=mask-pixels[i];
	}
}
//Timing function - from Advanced Computer Architecture - Coursework 2 - Prof. Kelly 
long stamp()
{
  struct timespec tv;
  long _stamp;
  clock_gettime(CLOCK_MONOTONIC, &tv);
  _stamp = tv.tv_sec * 1000 * 1000 * 1000 + tv.tv_nsec;
  return _stamp;
}

std::string LoadSource(const char *fileName)
{
	// Don't forget to change your_login here
	std::string baseDir="src/";
	if(getenv("HPCE_CL_SRC_DIR"))
	{
		baseDir=getenv("HPCE_CL_SRC_DIR");
	}
	
	std::string fullName=baseDir+"/"+fileName;
	
	std::ifstream src(fullName, std::ios::in | std::ios::binary);
	if(!src.is_open())
	{
		throw std::runtime_error("LoadSource : Couldn't load cl file from '"+fullName+"'.");
	}

	return std::string((std::istreambuf_iterator<char>(src)), // Node the extra brackets.
            std::istreambuf_iterator<char>()
		);
}

void process_cl(int levels, unsigned w, unsigned h, unsigned /*bits*/, std::vector<uint32_t> &pixels)
{
	std::vector<cl::Platform> platforms;

	cl::Platform::get(&platforms);
	if(platforms.size()==0)
	{
		throw std::runtime_error("No OpenCL platforms found.");
	}

    std::cerr<<"Found "<<platforms.size()<<" platforms\n";
	for(unsigned i=0;i<platforms.size();i++)
	{
		std::string vendor=platforms[0].getInfo<CL_PLATFORM_VENDOR>();
		std::cerr<<"  Platform "<<i<<" : "<<vendor<<"\n";
	}

    int selectedPlatform=0;
	if(getenv("HPCE_SELECT_PLATFORM"))
	{
		selectedPlatform=atoi(getenv("HPCE_SELECT_PLATFORM"));
	}
	std::cerr<<"Choosing platform "<<selectedPlatform<<"\n";
	cl::Platform platform=platforms.at(selectedPlatform);

    std::vector<cl::Device> devices;
	platform.getDevices(CL_DEVICE_TYPE_ALL, &devices);	
	if(devices.size()==0){
		throw std::runtime_error("No opencl devices found.\n");
	}
		
	std::cerr<<"Found "<<devices.size()<<" devices\n";
	for(unsigned i=0;i<devices.size();i++){
		std::string name=devices[i].getInfo<CL_DEVICE_NAME>();
		std::cerr<<"  Device "<<i<<" : "<<name<<"\n";
	}

	int selectedDevice=0;
	if(getenv("HPCE_SELECT_DEVICE")){
		selectedDevice=atoi(getenv("HPCE_SELECT_DEVICE"));
	}
	std::cerr<<"Choosing device "<<selectedDevice<<"\n";
	cl::Device device=devices.at(selectedDevice);

	cl::Context context(devices);

	std::string kernelSource=LoadSource("process_kernel.cl");
	
	cl::Program::Sources sources;	// A vector of (data,length) pairs
	sources.push_back(std::make_pair(kernelSource.c_str(), kernelSource.size()+1));	// push on our single string
	
	cl::Program program(context, sources);
	try
	{
		program.build(devices);
	}
	catch(...)
	{
		for(unsigned i=0;i<devices.size();i++)
		{
			std::cerr<<"Log for device "<<devices[i].getInfo<CL_DEVICE_NAME>()<<":\n\n";
			std::cerr<<program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(devices[i])<<"\n\n";
		}
		throw;
	}
/**************** Dunno about buffers**/
	// size_t cbBuffer=4*world.w*world.h;
	// cl::Buffer buffProperties(context, CL_MEM_READ_ONLY, cbBuffer);
	// cl::Buffer buffState(context, CL_MEM_READ_WRITE, cbBuffer);
	// cl::Buffer buffBuffer(context, CL_MEM_READ_WRITE, cbBuffer);

	cl::Kernel kernel(program, "process");
	
	unsigned w=world.w, h=world.h;
	
	float outer=world.alpha*dt;		// We spread alpha to other cells per time
	float inner=1-outer/4;				// Anything that doesn't spread stays
	
	kernel.setArg(0, levels);
	kernel.setArg(1, w);
	kernel.setArg(3, h);
	kernel.setArg(2, &pixels);
(int levels, unsigned w, unsigned h, unsigned /*bits*/, std::vector<uint32_t> &pixels
	cl::CommandQueue queue(context, device);
	
	queue.enqueueWriteBuffer(buffProperties, CL_TRUE, 0, cbBuffer, &world.properties[0]);

	// This is our temporary working space
	//std::vector<float> buffer(w*h);
	cl::NDRange offset(0, 0);				// Always start iterations at x=0, y=0
	cl::NDRange globalSize(1, levels);	// Global size must match the original loops
	cl::NDRange localSize=cl::NullRange;	// We don't care about local size

	queue.enqueueWriteBuffer(buffState, CL_TRUE, 0, cbBuffer, &world.state[0]); //not sure about this

	//cl::Event evCopiedState;		
		
	//std::vector<cl::Event> kernelDependencies(1, evCopiedState);
	// cl::Event evExecutedKernel;
	// std::vector<uint32_t> packed(w*h, 0);
	// for(uint32_t x = 0; x < w; x++)
	// {
	// 	for(uint32_t y = 0; y < h; y++)
	// 	{
	// 		unsigned index=y*w + x;
	// 		packed[index] = world.properties[index];
	// 		if(!((world.properties[index] & Cell_Fixed) || (world.properties[index] & Cell_Insulator)))
	// 		{
	// 				if(!(world.properties[index-w] & Cell_Insulator)) 
	// 				{
	// 					packed[index] +=0x4;
	// 				}
					
	// 				// Cell below
	// 				if(!(world.properties[index+w] & Cell_Insulator))
	// 				{
	// 					packed[index] +=0x8;
	// 				}
								
	// 					// Cell left
	// 				if(! (world.properties[index-1] & Cell_Insulator)) 
	// 				{
	// 					packed[index] +=0x10;
	// 				}
								
	// 					// Cell right
	// 				if(! (world.properties[index+1] & Cell_Insulator)) 
	// 				{
	// 					packed[index] +=0x20;
	// 				}	
	// 		}
	// 	}
	// }	
// 	for(unsigned t=0;t<n;t++)
// 	{
// 		queue.enqueueNDRangeKernel(kernel, offset, globalSize, localSize);
// 		queue.enqueueBarrier();	// <- new barrier here
	
// 		//std::swap(buffState, buffBuffer);
// 		queue.enqueueCopyBuffer(buffBuffer, buffState, 0,0, cbBuffer);
// 		//	kernel_xy(x,y,w, &world.state[0], &world.properties[0], &buffer[0], outer, inner);

// 		//std::vector<cl::Event> copyBackDependencies(1, evExecutedKernel);
		
// 		world.t += dt; // We have moved the world forwards in time
		
// 	} // end of for(t...
	
// 	queue.enqueueReadBuffer(buffBuffer, CL_TRUE, 0, cbBuffer, &world.state[0]);
// }



int main(int argc, char *argv[])
{
	long s1, s2, s3, s4, s5, s6, s7;
	try
	{
		if(argc<3)
		{
			fprintf(stderr, "Usage: process width height [bits] [levels]\n");
			fprintf(stderr, "   bits=8 by default\n");
			fprintf(stderr, "   levels=1 by default\n");
			exit(1);
		}
		
		unsigned w=atoi(argv[1]);
		unsigned h=atoi(argv[2]);
		
		unsigned bits=8;
		if(argc>3)
		{
			bits=atoi(argv[3]);
		}
		
		if(bits>32)
			throw std::invalid_argument("Bits must be <= 32.");
		
		unsigned tmp=bits;
		while(tmp!=1)
		{
			tmp>>=1;
			if(tmp==0)
				throw std::invalid_argument("Bits must be a binary power.");
		}
		
		if( ((w*bits)%64) != 0){
			throw std::invalid_argument(" width*bits must be divisible by 64.");
		}
		
		int levels=1;
		if(argc>4){
			levels=atoi(argv[4]);
		}
		
		fprintf(stderr, "Processing %d x %d image with %d bits per pixel.\n", w, h, bits);
		
		uint64_t cbRaw=uint64_t(w)*h*bits/8;
		std::vector<uint64_t> raw(cbRaw/8);
		
		std::vector<uint32_t> pixels(w*h);
		s1 = stamp();
		while(1)
		{
			//s3 = stamp();
			if(!read_blob(STDIN_FILENO, cbRaw, &raw[0]))
				break;	// No more images
			//s4 = stamp();
			unpack_blob(w, h, bits, &raw[0], &pixels[0]);		
			//s5 = stamp();
			process(levels, w, h, bits, pixels);
			//invert(levels, w, h, bits, pixels);
			//s6 = stamp();
			pack_blob(w, h, bits, &pixels[0], &raw[0]);
			//s7 = stamp();
			write_blob(STDOUT_FILENO, cbRaw, &raw[0]);			
		}
		s2 = stamp();
     	fprintf(stderr,"Overall time %g s\n", (s2 - s1) / 1e9);
		// fprintf(stderr,"Read Blob %g s\n", (s4 - s3) / 1e9);
		// fprintf(stderr,"unpack_blob %g s\n", (s5 - s4) / 1e9);
		// fprintf(stderr, "process %g s\n", (s6 - s5) / 1e9);
		// fprintf(stderr,"pack_blob %g s\n", (s7 - s6) / 1e9);
		// fprintf(stderr,"write_blob %g s\n", (s2 - s7) / 1e9);
		return 0;
	}catch(std::exception &e)
	{
		std::cerr<<"Caught exception : "<<e.what()<<"\n";
		return 1;
	}
}

