Running Instructions
=================================================================================================================================================================================================================================

The coursework source code is contained in src/

Test code is in src/test, a test script in script/ and the original provided source code in oldsrc/.

make all can be used to compile three binaries in bin/ process, the new implementation, oldproccess, the original code, and ip_test which runs the verification test suite of behaviour of image_process.

Behaviour can be verified quickly using

	`./script/test_program.sh`

This will iterate over input images of different bit depth, different numbers of levels and images of different sizes and assert that the output is bitwise identical.

New images can be converted to grayscale of variable bit depth using imageMagick:

	`convert <new_input>.<extension> -depth <depth> gray:<output>.raw`

This can then be run through the program using:

	`cat <output>.raw | ./bin/process <width> <height> <depth> <levels>`

The hex output can be obtained by piping to `hexdump`, which can then be diff'd against another to verify identical output (this is what program_test.sh does).

The output can be converted back to a viewable png using

	`cat <output>.raw | ./bin/process <width> <height> <depth> <levels> | convert -size <width>x<height> -depth <depth> gray:<(cat -) <output_filename>.png`

This can then be viewed using a tool such as 'eye of gnome' with:

	`eog <output_filename>.png`


=================================================================================================================================================================================================================================
HPCE Coursework 5
Streaming Morphological Filters

Introduction:
The purpose of this coursework was to gain an appreciation that there are two different types of "fast" which can be considered when thinking of high performance computing.

1. High Throughput: Work perfect for a GPU.  You have lots of computation that needs doing and you want the final result done as quickly as possible.  This is lots of calculation done as quickly as possible; you don't care about the time to process each individual computation.

2. Low latency:  In this work, you know you have a lot of work to be done, but you want each individual computation to be done as quickly as possible so you can have the result with minimal latency.  This is the nature of problem this coursework dealt with, i.e the performance metric is how quickly we produce the result for each pixel of the incoming image after we read the original pixel value in.

The Original Code:
The original program we were provided the code performed open and close operations on an input image; either a number of erodes followed by dilates, or dilates followed by erodes depending upon the levels argument passed to the program.

The input is a grayscale raw image with a number of bits per pixel represented as packed binary, in little-endian scanline order. For images with less than 8 bits per pixel, the left-most pixel is in the MSB.

In the original program, the entire image was read into to an array and then processed sequentially.  To calculate an erode, the program would iterate over every pixel in the image and calculate the minimum value of the pixels immediately surrounding, placing the result in an intermediate buffer.  The dilate is then performed on the result from the erode, producing the final output which can then be written out.

This is particularly inefficient and ultimately infeasible for particularly large images, as it will not be possible to store the entire image in a buffer which can then be operated upon.  It also requires 2*levels calls to erode/dilate to process the image, a very slow process for large values of levels.

Our Solution:
Our solution overcomes the limitations of the original program and boosts performance substantially, seeking to minimize latency between a pixel being read from the input stream and the result for that pixel being written out.

To do this, we read a "chunk" of 64 bits from the incoming image stream into a cyclic buffer.  When we have read enough incoming chunks to be able to begin processing the data, we perform the forward operation on the first chunk of data we read in, placing the result into an intermediate buffer the same size as the original pixel buffer.  Once we have processed enough of the original input to begin processing the intermediate buffer, we process a chunk of the intermediate buffer producing the final result for the incoming pixels in that chunk, and the result is written to the output stream.  This is shown below:

Input Stream -> Read Input Stream -> Original Pixel Buffer
Original Pixel Buffer -> Process Forward operation locally -> Intermediate Result Buffer
Intermediate Result Buffer -> Process Backward operation locally -> Output Stream

The size of the buffers is calculated as a function of the number of the image size, bits per pixel, and levels the program is being executed with.

The impact of Levels:

With levels > 1, it is important to understand the operation being performed.

Levels = 1 looks as follows:

	 ___
 __ |___|___
|___|___|___|
    |___|

If you have levels > 1, each surrounding 1 pixel, needs to be the result of the surrounding erodes/dilates to have the correct value for the subsequent opposite operation.  In real terms, this means the number of pixels which affect the final value for that operation grows outwards, as each pixel affecting the final, is affected by those surrounding.  Is it possible to unroll this dependency so it can be performed in a single step, resulting in the diamond pattern as shown for levels = 2 below, where x is the final resultant pixel being calculated.

         ___
     ___|___|___ 
 ___|___|___|___|___
|___|___|_x_|___|___|
    |___|___|___|
        |___|

From this it can be seen that the total number of pixel lines needed to perform a erode/dilate operation on any given pixel is (2*levels + 1) in any direction of the pixel.  A further complication is that having calculated using any given chunk, it will later be needed to compute the operation on any pixels within (2*layers) lines below.  As a result, once a chunk has been processed, it must be maintained until all those pixels depending upon it have further been calculated.  This lends itself ideally to a cyclic buffer.  We can subsequently calculate our required buffer sizes as a function of levels, width and height of image as will be shown below.

An example,

With a 32 x 32 image, 2 bits per pixel and levels = 1.

Each chunk is 64 bits, so with 2 bits per pixel, each chunk contains 32 pixels, an entire line of the image.

To calculate an erode/dilate, it is necessary to, for a pixel in the center of the image height, know the values of only the pixels immediately above and below.

Since levels = 1, the buffer size will therefore be 32 pixels * (2 * levels + 1).  This allows the buffer to contain the line above, the line of pixels being calculated, and the line below.

The intermediate buffer will be populated in the same way as chunks being being read and unpacked, and can be processed in the same way, one chunk (32 pixels at a time)


Cyclic buffers are maintained using 4 pointers as shown below for a 32x32 image buffer with levels = 1:

 ___ ___ ___ ___ ___ ___ ___           ___ ___ ___ ___          ___ ___ ___ ___ ___ ___          ___ ___ ___ ___ ___
|___|___|___|___|___|___|___|.........|___|___|___|___|........|___|___|___|___|___|___|........|___|___|___|___|___|
  ^                                     ^                        ^                                                     ^
  Buffer Start                         Process                  Insert                                                Buffer End

As a new chunk is read into the buffer, the pointer is incremented by the number of pixels per chunk and the chunk counter incremented.  Once the insert pointer goes out of bounds of the buffer by advancing to or beyond the Buffer End pointer, it is wrapped back around to the beginning, allowing the original data to be written over, effectively creating a FIFO (first in first out) data structure.

Trying to access the element directly above a pixel at address A can be calculated simply by the dx and dy to the required pixel with Result = *(A + dx + w*dy).  With the cyclic buffer, it is trivial to check that if a pixel is valid, i.e can be safely fetched, NOT x < 0 || x >= w for example, if the result address is outside the cyclic buffer, it is simple to wrap it around the beginning of end to find the correct value with pointer arithmetic.  e.g result > buffer end, result = buffer start + (result - buffer end)

Expanding the example above with a 32x32 image, 2 bits per pixel and levels = 1,

It can be seen that there is one chunk per line of image.  At the beginning, after reading one chunk:

Buffer Contents (pixels):

0....31
-....-
-....-

After 2 chunks

0....31
32...63
-....-

It can be seen that we are now ready to process chunk 1 and populate the first chunk of the intermediate buffer as the "line" above is out of bounds.  Subsequently, we can then process a new chunk following every new chunk read in.

After processing a chunk, the counter is incremented.

If we now look at the intermediate buffer too,

After 2 chunks read in.

0....31   ->    0....31
32...63         -....-
-....-          -....-

After 3 chunks read in.

0....31         0....31
32...63   ->    32...63
64...95         -....-

It can now be seen that we are ready to process the intermediate buffer contents to produce the final result which will be packed and written out

After 3 chunks read in, with final results buffer

0....31         0....31    ->    0...31
32...63   ->    32...63
64...95         -....-

It can be seen from this pattern that for this example, the final result will be two "chunks" latency.

This pattern also works if there are multiple chunks per line of original image.  With a 64x64 for example

0....31,32....63
64...95...

It can therefore be seen that processing latency for each stage is that (levels * chunks per line + 1) need to be read in to have enough data to begin processing from each buffer at each stage.

Once all chunks have been read in, the processing continues until all chunks have also been processed.  When all buffers have been fully processed, the image is finished and the program loops back to the beginning to try and start processing a new image from the input stream.


Optimising Processing:

Levels = 1

Streaming SIMD Extensions (SSE) is an SIMD instruction set extension to the x86 architecture that has been available since 1999.  It allows manual intrisic use of 128 bit vector arithmetic (used in this case for vector min and max operations).  Up to SSE 4, only floating point min and max operations are permitted and so these were chosed to try and enhance portability of the code.

The processing of a chunk is as follows with the 32x32, 2 bits per pixel, 1 level example:

take pointer to first pixel of chunk

for i {1..Pixels per chunk/4..4}
do
	Load initial pixels into vector using offet from first pixel pointer
	Load comparison pixel above into vector
	compare into initial pixel vector
	do below, left, right
	done
	increment first pixel pointer by 4
done
unpack pixel values from vector and save into target memory
increment chunks processed

It can be seen from this approach that this becomes an issue when 32 bits are used.  A 32 bit integer cannot be represented by a 32 bit float, and so special 32 bit versions of the code have been written which perform the same calculations using doubles instead of floats.  These functions are named <name>32() to make them obvious.  However the above method works perfectly for 16 bits and less as there are at least 4 pixels in any chunk.

Levels > 1

As shown earlier, for levels > 1, the pattern of access to surrounding pixels can be seen to be a diamond increasing with respect to levels:

         ___
     ___|___|___ 
 ___|___|___|___|___
|___|___|_x_|___|___|
    |___|___|___|
        |___|

Since access is needed to surrounding pixels in a pattern, it is necessary to find a way to calculate dx and dy to the target from the original pixel in order to fetch the target value from memory.

We observed that the diamond can be used to warp the access pattern in terms of i and j:

         ___
    i___|___|___ j
 ___|___|___|___|___
|___|___|_x_|___|___|
    |___|___|___|
        |___|

This can be seen to give

         ___
    i___|0,0|___ j
 ___|1,0|___|0,1|___
|2,0|___|1,1|___|0,2|
    |2,1|___|1,2|
        |2,2|

The mapping of dy can be shown to be -levels + i + j, i.e 0,0 gives dy = -2, two pixels above the original pixel.
The mapping of dx can be shown to be j - i, i.e dx = 0 for any point where i=j, a vertical column.

It can be seen here though that there is a pattern of elements which this solution does not visit.  We therefore calculated in terms of i,j and a third element k, giving the following for loop which visits the cells in the given pattern:

    for (int i = 0; i < levels + 1; i++)
    {
        for (int j = 0; j < levels + 1; j++)
        {
            squareCount++;
            // printf("dx: %d dy: %d\n", mapIJKtoDX(i, j, 0, levels), mapIJKtoDY(i, j, 0, levels));
            printf("i %d j %d k %d = %f\n", i, j, 0, MAXPIXIJK(7, 7, 3, 0, i, j, 0, levels, &diamond[0], &diamond[24], &diamond[48]));
            if (i == 0)
            {
                for (int k = 1; k < 2 * j; k += 2)
                {
                    squareCount++;
                    // printf("dx: %d dy: %d\n", mapIJKtoDX(i, j, k, levels), mapIJKtoDY(i, j, k, levels));
                    printf("i %d j %d k %d = %f\n", i, j, k, MAXPIXIJK(7, 7, 3, 0, i, j, k, levels, &diamond[0], &diamond[24], &diamond[48]));
                    // getPixel(7, 7, 3, 2, mapIJKtoDX(i, j, k, levels), mapIJKtoDY(i, j, k, levels), &diamond[0], &diamond[24], &diamond[48]
                }
            }
            else if (i < j && i != 0 && j == levels)
            {
                for (int k = 1; k < 2 * (levels - i); k += 2)
                {
                    squareCount++;
                    // printf("dx: %d dy: %d\n", mapIJKtoDX(i, j, k, levels), mapIJKtoDY(i, j, k, levels));
                    printf("i %d j %d k %d = %f\n", i, j, k, MAXPIXIJK(7, 7, 3, 0, i, j, k, levels, &diamond[0], &diamond[24], &diamond[48]));
                }
            }
        }
    }

             ___
         ___|_1_|___
     ___|11_|_3_|_2_|___
 ___|17_|_6_|12_|_5_|_4_|___
|22_|10_|18_|_9_|13_|_8_|_7_|
    |23_|16_|19_|15_|14_|
        |24_|21_|20_|
            |25_|


To calculate dx now, we calculate j - i - k, and dy, -levels + j + i as before.

This allows us to directly access all necessary elements systematically when performing the erode or dilate operation.


Testing:

To test our code as it was written, a test program was written which would effectively unit test the behaviours of all functions in image_process under known conditions and assert behaviour is as intended.
To test the final output is bit compliant a script was written which systematically tests images of different bit sizes, image sizes and levels.

Work Distribution:

All work in this project was split equally with alternations between pair programming when there were bugs to be squashed, and parallel work, one on writing functionality, the other testing to ensure behaviour as required.

We did whiteboard sessions to discuss the problem, different ideas for solutions and how to divide the workload and structure the final code.

We then split up to implement and test, and then worked in parallel, R. Evans writing this readme, and R. Bennett commenting and tidying the source code.