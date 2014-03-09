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
                for (int k = 1; k < 2 * (levels - i); k += 2)
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
    assert(!validPixel(w, h, w, h, 0, 0));
    assert(!validPixel(w, h, w / 2, h - 1, 5, 2));

    assert(validPixel(w, h, w / 2, h / 2, (w / 2) - 1, (h / 2) - 1));

    //Test validPixel and getPixel

    testStreamBuffer[0] = 50;
    testStreamBuffer[0 + w] = 100;

    unsigned x, y;

    calculateChunkXY(w, h, &x, &y, 128, chunksPerLine, pixPerChunk);

    assert(x == 0);
    assert(y == 1);

    assert(validPixel(w, h, x, y, 0, -1));
    assert(getPixel(w, h, x, y, 0, -1, &testStreamBuffer[0], &testStreamBuffer[512], &testStreamBuffer[1028]) == 50);

    testStreamBuffer[0] = 5;
    testStreamBuffer[1027] = 1;

    assert(getPixel(w,h,0,0,1,0,&testStreamBuffer[0], &testStreamBuffer[1027], &testStreamBuffer[1028]) == 5);


    // ERODE TEST TOP LEFT CORNER
    // 5 1 5 3 7....
    // 1 2 3 4....

    // Should give
    // 1 1 1 3 in results Buffer

    levels = 1;
    uint64_t chunksProcessed = 0; //Process top left corner
    uint64_t chunksRead = 128; // We've read full width + 1 chunk
    float *pixCalculate = &testStreamBuffer[0];

    float *processedResultsBuffer = (float *)malloc(sizeof(float) * pixPerChunk);

    testStreamBuffer[0] = 5;
    testStreamBuffer[1] = 1;
    testStreamBuffer[2] = 5;
    testStreamBuffer[3] = 3;
    testStreamBuffer[4] = 7;
    testStreamBuffer[w] = 1;
    testStreamBuffer[w + 1] = 2;
    testStreamBuffer[w + 2] = 3;
    testStreamBuffer[w + 3] = 4;

    calculateChunkXY(w, h, &x, &y, chunksProcessed, chunksPerLine, pixPerChunk);
    assert(x == 0);
    assert(y == 0);

    //Test correct Pixels are fetched
    assert(getPixel(w, h, 0, 0, 0, 1, &testStreamBuffer[0], &testStreamBuffer[0], &testStreamBuffer[1028]) == 1);
    assert(getPixel(w, h, 1, 0, 0, 1, &testStreamBuffer[0], &testStreamBuffer[1], &testStreamBuffer[1028]) == 2);
    assert(getPixel(w, h, 2, 0, 0, 1, &testStreamBuffer[0], &testStreamBuffer[2], &testStreamBuffer[1028]) == 3);
    assert(getPixel(w, h, 3, 0, 0, 1, &testStreamBuffer[0], &testStreamBuffer[3], &testStreamBuffer[1028]) == 4);

    //Test the erode!
    erodeChunk(w, h, levels, pixPerChunk, chunksPerLine, &chunksProcessed, &testStreamBuffer[0], pixCalculate, &testStreamBuffer[1027], processedResultsBuffer);

    assert(processedResultsBuffer[0] == 1);
    assert(processedResultsBuffer[1] == 1);
    assert(processedResultsBuffer[2] == 1);
    assert(processedResultsBuffer[3] == 3);

    assert(chunksProcessed == 1);

    // DILATE TEST TOP LEFT CORNER
    // 5 1 5 3 7....
    // 1 2 3 4....

    // Should give
    // 5 5 5 7 in results Buffer

    chunksProcessed = 0;

    calculateChunkXY(w, h, &x, &y, chunksProcessed, chunksPerLine, pixPerChunk);
    assert(x == 0);
    assert(y == 0);

    assert(getPixel(w, h, 0, 0, 0, 1, &testStreamBuffer[0], &testStreamBuffer[0], &testStreamBuffer[1028]) == 1);
    assert(getPixel(w, h, 1, 0, 0, 1, &testStreamBuffer[0], &testStreamBuffer[1], &testStreamBuffer[1028]) == 2);
    assert(getPixel(w, h, 2, 0, 0, 1, &testStreamBuffer[0], &testStreamBuffer[2], &testStreamBuffer[1028]) == 3);
    assert(getPixel(w, h, 3, 0, 0, 1, &testStreamBuffer[0], &testStreamBuffer[3], &testStreamBuffer[1028]) == 4);

    dilateChunk(w, h, levels, pixPerChunk, chunksPerLine, &chunksProcessed, &testStreamBuffer[0], pixCalculate, &testStreamBuffer[1027], processedResultsBuffer);

    assert(processedResultsBuffer[0] == 5);
    assert(processedResultsBuffer[1] == 5);
    assert(processedResultsBuffer[2] == 5);
    assert(processedResultsBuffer[3] == 7);

    assert(chunksProcessed == 1);

    //ERODE CENTER LEFT TEST
    // 7 3 7 3
    // 2 8 9 10 9
    // 5 6 7 8

    // Should give
    // 2 2 7 3

    testStreamBuffer[0] = 7;
    testStreamBuffer[1] = 3;
    testStreamBuffer[2] = 7;
    testStreamBuffer[3] = 3;
    testStreamBuffer[w] = 2;
    testStreamBuffer[w + 1] = 8;
    testStreamBuffer[w + 2] = 9;
    testStreamBuffer[w + 3] = 10;
    testStreamBuffer[w + 4] = 9;
    testStreamBuffer[2*w] = 5;
    testStreamBuffer[2*w + 1] = 6;
    testStreamBuffer[2*w + 2] = 7;
    testStreamBuffer[2*w + 3] = 8;

    chunksProcessed = 128;

    calculateChunkXY(w, h, &x, &y, chunksProcessed, chunksPerLine, pixPerChunk);
    assert(x == 0);
    assert(y == 1);

    assert(testStreamBuffer[515] == 10);

    //Above
    assert(getPixel(w, h, x, y, 0, -1, &testStreamBuffer[0], &testStreamBuffer[512], &testStreamBuffer[1028]) == 7);
    assert(getPixel(w, h, x+1, y, 0, -1, &testStreamBuffer[0], &testStreamBuffer[513], &testStreamBuffer[1028]) == 3);
    assert(getPixel(w, h, x+2, y, 0, -1, &testStreamBuffer[0], &testStreamBuffer[514], &testStreamBuffer[1028]) == 7);
    assert(getPixel(w, h, x+3, y, 0, -1, &testStreamBuffer[0], &testStreamBuffer[515], &testStreamBuffer[1028]) == 3);

    //Below
    assert(getPixel(w, h, x, y, 0, 1, &testStreamBuffer[0], &testStreamBuffer[512], &testStreamBuffer[1028]) == 5);
    assert(getPixel(w, h, x+1, y, 0, 1, &testStreamBuffer[0], &testStreamBuffer[513], &testStreamBuffer[1028]) == 6);
    assert(getPixel(w, h, x+2, y, 0, 1, &testStreamBuffer[0], &testStreamBuffer[514], &testStreamBuffer[1028]) == 7);
    assert(getPixel(w, h, x+3, y, 0, 1, &testStreamBuffer[0], &testStreamBuffer[515], &testStreamBuffer[1028]) == 8);

    assert(testStreamBuffer[512] == 2);

    erodeChunk(w, h, levels, pixPerChunk, chunksPerLine, &chunksProcessed, &testStreamBuffer[0], &testStreamBuffer[512], &testStreamBuffer[1027], processedResultsBuffer);

    assert(processedResultsBuffer[0] == 2);
    assert(processedResultsBuffer[1] == 2);
    assert(processedResultsBuffer[2] == 7);
    assert(processedResultsBuffer[3] == 3);

    assert(chunksProcessed == 129);


    w = 32;
    h = 32;
    bits = 2;
    levels = 1;

    bytesToRead = sizeof(uint64_t);
    assert(bytesToRead == 8);

    pixPerChunk = 64 / bits;
    assert(pixPerChunk == 32);

    chunksPerLine = w / pixPerChunk;
    assert(chunksPerLine == 1);

    totalChunks = (w * h) / pixPerChunk;
    assert(totalChunks == 32);

    pixBufSize = sizeof(float) * (w * ((2 * levels) + 1));
    assert(pixBufSize == (3 * 32 * sizeof(float)));

    free(testStreamBuffer);

    testStreamBuffer = (float *)malloc(pixBufSize);

    for (int i = 0; i < pixBufSize/sizeof(float); i++)
    {
        testStreamBuffer[i] = i+1;
    }


    //Do some tests;

    //GetChunkXY
    chunksProcessed = 0;
    calculateChunkXY(w, h, &x, &y, chunksProcessed, chunksPerLine, pixPerChunk);
    assert(x == 0 && y == 0);

    chunksProcessed = 1;
    calculateChunkXY(w, h, &x, &y, chunksProcessed, chunksPerLine, pixPerChunk);
    assert(x == 0 && y == 1);

    chunksProcessed = 2;
    calculateChunkXY(w, h, &x, &y, chunksProcessed, chunksPerLine, pixPerChunk);
    assert(x == 0 && y == 2);

    chunksProcessed = 3;
    calculateChunkXY(w, h, &x, &y, chunksProcessed, chunksPerLine, pixPerChunk);
    assert(x == 0 && y == 3);

    //Valid Pixel
    chunksProcessed = 0;
    calculateChunkXY(w, h, &x, &y, chunksProcessed, chunksPerLine, pixPerChunk);
    assert(!validPixel(w, h, x, y, 0, -1));
    assert(!validPixel(w, h, x, y, -1, 0));
    assert(!validPixel(w, h, x, y, -1, -1));
    assert(validPixel(w, h, x, y, 0, 0));
    assert(validPixel(w, h, x, y, 0, 1));
    assert(validPixel(w, h, x, y, 1, 0));

    //Top Right
    assert(validPixel(w, h, x + 31, y, -1, 0));
    assert(!validPixel(w, h, x + 31, y, 0, -1));
    assert(!validPixel(w, h, x + 31, y, -1, -1));
    assert(!validPixel(w, h, x + 31, y, 1, 0));
    assert(validPixel(w, h, x + 31, y, -1, 1));









    printf("All Tests Run Successfully\n");


}