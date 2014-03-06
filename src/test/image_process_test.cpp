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
#include <cstring>

#include <assert.h>

#include "../image_process.hpp"

int main(int argc, char *argv[])
{

    //SET UP STUFF WE NEED

    //PARAMS
    unsigned w = 512;
    unsigned h = 512;
    unsigned bits = 2;
    int levels = 1;

    uint32_t bytesToRead = 0;
    uint32_t bitMask = 0;
    uint32_t pixPerChunk = 0;

    switch (bits)
    {
    case 1:
        bytesToRead = 1;
        bitMask = 0x01;
        pixPerChunk = 8;
        break;
    case 2:
        bytesToRead = 1;
        bitMask = 0x03;
        pixPerChunk = 4;
        break;
    case 4:
        bytesToRead = 2;
        bitMask = 0x0F;
        pixPerChunk = 4;
        break;
    case 8:
        bytesToRead = 4;
        bitMask = 0xFF;
        pixPerChunk = 4;
        break;
    case 16:
        bytesToRead = 8;
        bitMask = 0xFFFF;
        pixPerChunk = 4;
        break;
    case 32:
        bytesToRead = 16;
        bitMask = 0xFFFFFFFF;
        pixPerChunk = 4;
        break;
    }

    uint32_t chunksPerLine = w / pixPerChunk;
    //512 pixels in a line, 4 pixels to a chunk
    assert(chunksPerLine == 512 / 4);
    uint32_t totalChunks = w * h / pixPerChunk;
    //512 columns, 512 rows, 4 pixels per chunk
    assert(totalChunks = 512 * 512 / 4);

    uint32_t pixBufSize = sizeof(float) * ((w * (2 * levels)) + pixPerChunk);

    //For levels = 1, need maximum of 2 full lines, plus 1 chunk;
    assert(pixBufSize / sizeof(float) == 1028);

    float *testStreamBuffer  = (float *)malloc(pixBufSize);

    levels = 3;
    printf("Levels = 3 - 2\n");
    int squareCount = 0;
    for (int i = 0; i < levels + 1; i++)
    {
        for (int j = 0; j < levels + 1; j++)
        {
            squareCount++;
            printf("dx: %d dy: %d\n", mapIJKtoDX(i, j, 0, levels), mapIJKtoDY(i, j, 0, levels));
            if (i == 0)
            {
                for (int k = 1; k < 2 * j; k += 2)
                {
                    squareCount++;
                    printf("dx: %d dy: %d\n", mapIJKtoDX(i, j, k, levels), mapIJKtoDY(i, j, k, levels));
                }
            }
            else if (i < j && i != 0 && j == levels)
            {
            	for (int k = 1; k < 2*(levels - i); k += 2)
                {
                    squareCount++;
                    printf("dx: %d dy: %d\n", mapIJKtoDX(i, j, k, levels), mapIJKtoDY(i, j, k, levels));
                }
            }
        }
    }

    //Should get 25
    printf("%d\n", squareCount);

    //             ___
    //         ___|_1_|___
    //     ___|11_|_3_|_2_|___
    // ___|17_|_6_|12_|_5_|_4_|___
    //|22_|10_|18_|_9_|13_|_8_|_7_|
    //    |23_|16_|19_|15_|14_|
    //        |24_|21_|20_|
    //            |25_|
    //             ___
    //         ___|___|___
    //     ___|___|___|___|___
    // ___|___|___|___|___|___|___
    //|___|___|___|___|___|___|___|
    //    |___|___|___|___|___|
    //        |___|___|___|
    //            |___|
    //Test valid

    levels = 2;

    //TEST VALID PIXEL
    assert(!validPixel(w, h, 0, 0, -1, -1));
    assert(!validPixel(w,h,w,h,0,0));
    assert(!validPixel(w,h,w/2,h-1,5,2));

   	assert(validPixel(w,h,w/2,h/2,(w/2) - 1,(h/2) -1));

   	//Test validPixel and getPixel

   	testStreamBuffer[0] = 50;
   	testStreamBuffer[0 + w] = 100;

   	unsigned x, y;

   	calculateChunkXY(w, h, &x, &y, 128, chunksPerLine, pixPerChunk);

   	assert(x == 0);
   	assert(y == 1);

   	assert(validPixel(w,h,x,y,0,-1));
   	printf("%f\n", testStreamBuffer[0]);
   	printf("%f\n", testStreamBuffer[512]);
   	printf("%f\n", testStreamBuffer[1027]);
   	assert(getPixel(w, h, x, y, 0, -1, &testStreamBuffer[0], &testStreamBuffer[512], &testStreamBuffer[1027]) == 50);




    printf("All Tests Run Successfully\n");


}