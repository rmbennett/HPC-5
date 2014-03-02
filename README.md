HPC-5
=====

High Performance Computing Coursework 5

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


USAGE*****

How to build ***

g++ -I include/ src/process.cpp -std=c++11 -lOpenCL

How to run ***
cat input.raw | ./a.out 512 512 2 | convert -size 512x512 -depth 2 gray:- output.png
Processing 512 x 512 image with 2 bits per pixel.
0.0644059 s


This part is pretty important 


The metric used to evaluate this work is maximum pixel
latency. Pixel latency is defined as the time between
a given pixel entering the program, and the transformed
pixel at the same co-ordinates leaving the program. The
goal is to minimise the maximimum latency over all
pixels. Latency measuring does not start until the first
pixel enters the pipeline, so performance measurement
is "on-hold" till that point. However, your program
must eventually read the first pixel...


So essentially any preprocessing can be done before pixels arrive. 


cat input1024.raw | ./a.out 1024 1024 2 | convert -size 1024x1024 -depth 2 gray:- output.png
Processing 1024 x 1024 image with 2 bits per pixel.
Overall time 0.329756 s
Read Blob 416990 s
unpack_blob 0.0132685 s
process 0.291943 s
pack_blob 0.00809472 s
write_blob 0.0161225 s


So we should work on process() - which contains predominantly lots of erodes and dilates. 




