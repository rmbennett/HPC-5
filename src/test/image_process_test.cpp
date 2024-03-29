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

#define FLOATMIN std::numeric_limits<float>::min()
#define FLOATMAX std::numeric_limits<float>::max()

#define MAXPIX(w,h,x,y,dx,dy,memStart,pPtr,memEnd) validPixel(w,h,x,y,dx,dy)?getPixel(w,h,x,y,dx,dy,memStart,pPtr,memEnd):FLOATMAX

#define MINPIX(w,h,x,y,dx,dy,memStart,pPtr,memEnd) validPixel(w,h,x,y,dx,dy)?getPixel(w,h,x,y,dx,dy,memStart,pPtr,memEnd):FLOATMIN

#define MAXPIXIJK(w,h,x,y,i,j,k,levels,memStart,pPtr,memEnd) validPixel(w,h,x,y,mapIJKtoDX(i,j,k,levels),mapIJKtoDY(i,j,k,levels))?getPixel(w,h,x,y,mapIJKtoDX(i,j,k,levels),mapIJKtoDY(i,j,k,levels),memStart,pPtr,memEnd):FLOATMAX

#define MINPIXIJK(w,h,x,y,i,j,k,levels,memStart,pPtr,memEnd) validPixel(w,h,x,y,mapIJKtoDX(i,j,k,levels),mapIJKtoDY(i,j,k,levels))?getPixel(w,h,x,y,mapIJKtoDX(i,j,k,levels),mapIJKtoDY(i,j,k,levels),memStart,pPtr,memEnd):FLOATMIN

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

    float *diamond = (float *)malloc(sizeof(float) * (2 * levels + 1));
    for (int i = 0; i < (2 * levels + 1); i++)
    {
        for (int j = 0; j < (2 * levels + 1); j++)
        {
            diamond[(i * (2 * levels + 1)) + j] = (i * (2 * levels + 1)) + j;
        }
    }

    printf("Levels = 3 - 2\n");

    //             ___
    // 0   1   2__|3__|4__ 5   6
    // 7   8__|9__|10_|11_|12_ 13
    // 14_|15_|16_|17_|18_|19_|20_
    //|21_|22_|23_|24_|25_|26_|27_|
    // 28 |29_|30_|31_|32_|33_|34
    // 35  36 |37_|38_|39_|40  41
    // 42  43  44 |45_|46  47  48

    fprintf(stderr, "\n             ___\n");
    fprintf(stderr, " 0   1   2__|3__|4__ 5   6\n");
    fprintf(stderr, " 7   8__|9__|10_|11_|12_ 13\n");
    fprintf(stderr, " 14_|15_|16_|17_|18_|19_|20_\n");
    fprintf(stderr, "|21_|22_|23_|24_|25_|26_|27_|\n");
    fprintf(stderr, " 28 |29_|30_|31_|32_|33_|34\n");
    fprintf(stderr, " 35  36 |37_|38_|39_|40  41\n");
    fprintf(stderr, " 42  43  44 |45_|46  47  48\n\n");

    int squareCount = 0;
    for (int i = 0; i < levels + 1; i++)
    {
        for (int j = 0; j < levels + 1; j++)
        {
            squareCount++;
            // printf("dx: %d dy: %d\n", mapIJKtoDX(i, j, 0, levels), mapIJKtoDY(i, j, 0, levels));
            printf("i %d j %d k %d = %f\n", i, j, 0, MAXPIXIJK(7, 7, 3, 2, i, j, 0, levels, &diamond[0], &diamond[24], &diamond[48]));
            if (i == 0)
            {
                for (int k = 1; k < 2 * j; k += 2)
                {
                    squareCount++;
                    // printf("dx: %d dy: %d\n", mapIJKtoDX(i, j, k, levels), mapIJKtoDY(i, j, k, levels));
                    printf("i %d j %d k %d = %f\n", i, j, k, MAXPIXIJK(7, 7, 3, 2, i, j, k, levels, &diamond[0], &diamond[24], &diamond[48]));
                    // getPixel(7, 7, 3, 2, mapIJKtoDX(i, j, k, levels), mapIJKtoDY(i, j, k, levels), &diamond[0], &diamond[24], &diamond[48]
                }
            }
            else if (i < j && i != 0 && j == levels)
            {
                for (int k = 1; k < 2 * (levels - i); k += 2)
                {
                    squareCount++;
                    // printf("dx: %d dy: %d\n", mapIJKtoDX(i, j, k, levels), mapIJKtoDY(i, j, k, levels));
                    printf("i %d j %d k %d = %f\n", i, j, k, MAXPIXIJK(7, 7, 3, 2, i, j, k, levels, &diamond[0], &diamond[24], &diamond[48]));
                }
            }
        }
    }

    fprintf(stderr, "Test if top is cropped... Should be no 3\n\n");

    squareCount = 0;
    for (int i = 0; i < levels + 1; i++)
    {
        for (int j = 0; j < levels + 1; j++)
        {
            squareCount++;
            // printf("dx: %d dy: %d\n", mapIJKtoDX(i, j, 0, levels), mapIJKtoDY(i, j, 0, levels));
            printf("i %d j %d k %d = %f\n", i, j, 0, MAXPIXIJK(7, 7, 3, 2, i, j, 0, levels, &diamond[0], &diamond[24], &diamond[48]));
            if (i == 0)
            {
                for (int k = 1; k < 2 * j; k += 2)
                {
                    squareCount++;
                    // printf("dx: %d dy: %d\n", mapIJKtoDX(i, j, k, levels), mapIJKtoDY(i, j, k, levels));
                    printf("i %d j %d k %d = %f\n", i, j, k, MAXPIXIJK(7, 7, 3, 2, i, j, k, levels, &diamond[0], &diamond[24], &diamond[48]));
                    // getPixel(7, 7, 3, 2, mapIJKtoDX(i, j, k, levels), mapIJKtoDY(i, j, k, levels), &diamond[0], &diamond[24], &diamond[48]
                }
            }
            else if (i < j && i != 0 && j == levels)
            {
                for (int k = 1; k < 2 * (levels - i); k += 2)
                {
                    squareCount++;
                    // printf("dx: %d dy: %d\n", mapIJKtoDX(i, j, k, levels), mapIJKtoDY(i, j, k, levels));
                    printf("i %d j %d k %d = %f\n", i, j, k, MAXPIXIJK(7, 7, 3, 2, i, j, k, levels, &diamond[0], &diamond[24], &diamond[48]));
                }
            }
        }
    }

    fprintf(stderr, "Test missing top half completely\n");

    squareCount = 0;
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

    assert(getPixel(w, h, 0, 0, 1, 0, &testStreamBuffer[0], &testStreamBuffer[1027], &testStreamBuffer[1028]) == 5);


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
    testStreamBuffer[2 * w] = 5;
    testStreamBuffer[2 * w + 1] = 6;
    testStreamBuffer[2 * w + 2] = 7;
    testStreamBuffer[2 * w + 3] = 8;

    chunksProcessed = 128;

    calculateChunkXY(w, h, &x, &y, chunksProcessed, chunksPerLine, pixPerChunk);
    assert(x == 0);
    assert(y == 1);

    assert(testStreamBuffer[515] == 10);

    //Above
    assert(getPixel(w, h, x, y, 0, -1, &testStreamBuffer[0], &testStreamBuffer[512], &testStreamBuffer[1028]) == 7);
    assert(getPixel(w, h, x + 1, y, 0, -1, &testStreamBuffer[0], &testStreamBuffer[513], &testStreamBuffer[1028]) == 3);
    assert(getPixel(w, h, x + 2, y, 0, -1, &testStreamBuffer[0], &testStreamBuffer[514], &testStreamBuffer[1028]) == 7);
    assert(getPixel(w, h, x + 3, y, 0, -1, &testStreamBuffer[0], &testStreamBuffer[515], &testStreamBuffer[1028]) == 3);

    //Below
    assert(getPixel(w, h, x, y, 0, 1, &testStreamBuffer[0], &testStreamBuffer[512], &testStreamBuffer[1028]) == 5);
    assert(getPixel(w, h, x + 1, y, 0, 1, &testStreamBuffer[0], &testStreamBuffer[513], &testStreamBuffer[1028]) == 6);
    assert(getPixel(w, h, x + 2, y, 0, 1, &testStreamBuffer[0], &testStreamBuffer[514], &testStreamBuffer[1028]) == 7);
    assert(getPixel(w, h, x + 3, y, 0, 1, &testStreamBuffer[0], &testStreamBuffer[515], &testStreamBuffer[1028]) == 8);

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

    for (int i = 0; i < pixBufSize / sizeof(float); i++)
    {
        testStreamBuffer[i] = i + 1;
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

    //First Line
    chunksProcessed = 0;
    calculateChunkXY(w, h, &x, &y, chunksProcessed, chunksPerLine, pixPerChunk);

    assert(x == 0 && y == 0);

    //Top left
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
    assert(!validPixel(w, h, x + 31, y, 1, -1));
    assert(validPixel(w, h, x + 31, y, -1, 1));

    //Left Edge
    for (int i = 0; i < h; i++)
    {
        assert(validPixel(w, h, x, y + i, 1, 0));
        assert(!validPixel(w, h, x, y + i, -1, 0));
    }

    //Right Edge
    for (int i = 0; i < h; i++)
    {
        assert(validPixel(w, h, x + 31, y + i, -1, 0));
        assert(!validPixel(w, h, x + 31, y + i, 1, 0));
    }

    //Along top
    for (int i = 0; i < w; i++)
    {
        assert(validPixel(w, h, x + i, y, 0, 1));
        assert(!validPixel(w, h, x + i, y, 0, -1));
    }

    chunksProcessed = 31;
    calculateChunkXY(w, h, &x, &y, chunksProcessed, chunksPerLine, pixPerChunk);

    assert(x == 0 && y == 31);

    //Bottom left
    assert(validPixel(w, h, x, y, 0, -1));
    assert(validPixel(w, h, x, y, 1, 0));
    assert(!validPixel(w, h, x, y, -1, 0));
    assert(!validPixel(w, h, x, y, -1, -1));
    assert(validPixel(w, h, x, y, 1, -1));
    assert(!validPixel(w, h, x, y, 0, 1));
    assert(!validPixel(w, h, x, y, 1, 1));

    //Bottom Right
    assert(!validPixel(w, h, x + 31, y, 1, 0));
    assert(!validPixel(w, h, x + 31, y, 0, 1));
    assert(!validPixel(w, h, x + 31, y, 1, 1));
    assert(validPixel(w, h, x + 31, y, -1, 0));
    assert(validPixel(w, h, x + 31, y, 0, -1));
    assert(validPixel(w, h, x + 31, y, -1, -1));

    //Along Bottom
    for (int i = 0; i < w; i++)
    {
        assert(validPixel(w, h, x + i, y, 0, -1));
        assert(!validPixel(w, h, x + i, y, 0, 1));
    }

    //Test getting Values...
    chunksProcessed = 0;
    calculateChunkXY(w, h, &x, &y, chunksProcessed, chunksPerLine, pixPerChunk);
    for (int i = 0; i < w; i++)
    {
        assert(getPixel(w, h, x, y, 0, 1, &testStreamBuffer[0], &testStreamBuffer[i], &testStreamBuffer[96]) == w + 1 + i);
    }

    chunksProcessed = 3;
    calculateChunkXY(w, h, &x, &y, chunksProcessed, chunksPerLine, pixPerChunk);
    assert(x == 0 && y == 3);
    for (int i = 0; i < w; i++)
    {
        assert(getPixel(w, h, x, y, 0, 1, &testStreamBuffer[0], &testStreamBuffer[(2 * w) + i], &testStreamBuffer[96]) == i + 1);
    }

    for (int i = 0; i < (w - 1); i++)
    {
        assert(getPixel(w, h, x, y, 1, 1, &testStreamBuffer[0], &testStreamBuffer[(2 * w) + i], &testStreamBuffer[96]) == i + 2);
    }

    printf("All Tests Run Successfully\n");

    float *testResults = (float *)malloc(sizeof(float) * pixPerChunk * chunksPerLine);

    assert(w == 32 && h == 32);
    assert(pixPerChunk == 32);
    assert(chunksPerLine == 1);


    //Top Row
    chunksProcessed = 0;

    erodeChunk(w, h, levels, pixPerChunk, chunksPerLine, &chunksProcessed, &testStreamBuffer[0], &testStreamBuffer[0], &testStreamBuffer[96], testResults);

    fprintf(stderr, "Erode Top\n");
    for (int i = 0; i < (pixPerChunk * chunksPerLine); i++)
    {
        fprintf(stderr, "Value %d %lf\n", i, testResults[i]);
    }

    chunksProcessed = 0;

    dilateChunk(w, h, levels, pixPerChunk, chunksPerLine, &chunksProcessed, &testStreamBuffer[0], &testStreamBuffer[0], &testStreamBuffer[96], testResults);

    fprintf(stderr, "Dilate Top\n");
    for (int i = 0; i < (pixPerChunk * chunksPerLine); i++)
    {
        fprintf(stderr, "Value %d %lf\n", i, testResults[i]);
    }


    //Middle Row
    erodeChunk(w, h, levels, pixPerChunk, chunksPerLine, &chunksProcessed, &testStreamBuffer[0], &testStreamBuffer[32], &testStreamBuffer[96], testResults);

    fprintf(stderr, "Erode Middle\n");
    for (int i = 0; i < (pixPerChunk * chunksPerLine); i++)
    {
        fprintf(stderr, "Value %d %lf\n", i, testResults[i]);
    }

    chunksProcessed = 1;

    dilateChunk(w, h, levels, pixPerChunk, chunksPerLine, &chunksProcessed, &testStreamBuffer[0], &testStreamBuffer[32], &testStreamBuffer[96], testResults);

    fprintf(stderr, "Dilate Middle\n");
    for (int i = 0; i < (pixPerChunk * chunksPerLine); i++)
    {
        fprintf(stderr, "Value %d %lf\n", i, testResults[i]);
    }

    //Bottom Row wrap

    erodeChunk(w, h, levels, pixPerChunk, chunksPerLine, &chunksProcessed, &testStreamBuffer[0], &testStreamBuffer[64], &testStreamBuffer[96], testResults);

    fprintf(stderr, "Erode Bottom\n");
    for (int i = 0; i < (pixPerChunk * chunksPerLine); i++)
    {
        fprintf(stderr, "Value %d %lf\n", i, testResults[i]);
    }

    chunksProcessed = 2;

    dilateChunk(w, h, levels, pixPerChunk, chunksPerLine, &chunksProcessed, &testStreamBuffer[0], &testStreamBuffer[64], &testStreamBuffer[96], testResults);

    fprintf(stderr, "Dilate Bottom\n");
    for (int i = 0; i < (pixPerChunk * chunksPerLine); i++)
    {
        fprintf(stderr, "Value %d %lf\n", i, testResults[i]);
    }

    //Top Row wrap
    erodeChunk(w, h, levels, pixPerChunk, chunksPerLine, &chunksProcessed, &testStreamBuffer[0], &testStreamBuffer[0], &testStreamBuffer[96], testResults);

    fprintf(stderr, "Erode Top Wrap\n");
    for (int i = 0; i < (pixPerChunk * chunksPerLine); i++)
    {
        fprintf(stderr, "Value %d %lf\n", i, testResults[i]);
    }

    chunksProcessed = 3;

    dilateChunk(w, h, levels, pixPerChunk, chunksPerLine, &chunksProcessed, &testStreamBuffer[0], &testStreamBuffer[0], &testStreamBuffer[96], testResults);

    fprintf(stderr, "Dilate Top Wrap\n");
    for (int i = 0; i < (pixPerChunk * chunksPerLine); i++)
    {
        fprintf(stderr, "Value %d %lf\n", i, testResults[i]);
    }

}