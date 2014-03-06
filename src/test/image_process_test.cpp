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

    //Test map i,j to dx, dy for levels = 2 as shown below (not used for levels = 1)
    //          ___
    //     i___|___|___ j
    //  ___|___|___|___|___
    // |___|___|___|___|___|
    //     |___|___|___|
    //         |___|

    levels = 2;
    printf("Levels = 2\n");
    for (int w = 0; w < levels + 1; w++)
    {
        for (int x = 0; x < levels + 1; x++)
        {
            printf("dx: %d dy: %d\n", mapIJtoDX(w, x, levels), mapIJtoDY(w, x, levels));
        }
    }

    /*
    0 dx: 0 dy: -2
    1 dx: 1 dy: -1
    2 dx: 2 dy: 0
    3 dx: -1 dy: -1
    4 dx: 0 dy: 0
    5 dx: 1 dy: 1
    6 dx: -2 dy: 0
    7 dx: -1 dy: 1
    8 dx: 0 dy: 2 */

    //          ___
    //     i___|_0_|___ j
    //  ___|_3_|___|_1_|___
    // |_6_|___|_4_|___|_2_|
    //     |_7_|___|_5_|
    //         |_8_|

    int i = 0;
    int j = 0;
    //          ___
    //     i___|_X_|___ j
    //  ___|___|___|___|___
    // |___|___|___|___|___|
    //     |___|___|___|
    //         |___|

    assert(mapIJtoDX(i, j, levels) == 0);
    assert(mapIJtoDY(i, j, levels) == -2);

    i = 1;
    j = 1;
    //          ___
    //     i___|___|___ j
    //  ___|___|___|___|___
    // |___|___|_X_|___|___|
    //     |___|___|___|
    //         |___|

    assert(mapIJtoDX(i, j, levels) == 0);
    assert(mapIJtoDY(i, j, levels) == 0);

    i = 1;
    j = 2;
    //          ___
    //     i___|___|___ j
    //  ___|___|___|___|___
    // |___|___|___|___|___|
    //     |___|___|_X_|
    //         |___|

    assert(mapIJtoDX(i, j, levels) == 1);
    assert(mapIJtoDY(i, j, levels) == 1);


    levels = 3;

    printf("Levels = 3\n");
    for (int w = 0; w < levels + 1; w++)
    {
        for (int x = 0; x < levels + 1; x++)
        {
            printf("dx: %d dy: %d\n", mapIJtoDX(w, x, levels), mapIJtoDY(w, x, levels));
        }
    }

    // 0 dx: 0 dy: -3
    // 1 dx: 1 dy: -2
    // 2 dx: 2 dy: -1
    // 3 dx: 3 dy: 0
    // 4 dx: -1 dy: -2
    // 5 dx: 0 dy: -1
    // 6 dx: 1 dy: 0
    // 7 dx: 2 dy: 1
    // 8 dx: -2 dy: -1
    // 9 dx: -1 dy: 0
    // 10 dx: 0 dy: 1
    // 11 dx: 1 dy: 2
    // 12 dx: -3 dy: 0
    // 13 dx: -2 dy: 1
    // 14 dx: -1 dy: 2
    // 15 dx: 0 dy: 3

    //             ___
    //         ___|_0_|___
    //     ___|_4_|___|_1_|___
    // ___|_8_|___|_5_|___|_2_|___
    //|12_|___|_9_|___|_6_|___|_3_|
    //    |13_|___|10_|___|_7_|
    //        |14_|___|11_|
    //            |15_|


    printf("Levels = 3 - 2\n");
    int squareCount = 0;
    for (int w = 0; w < levels + 1; w++)
    {
        for (int x = 0; x < levels + 1; x++)
        {
            squareCount++;
            printf("dx: %d dy: %d\n", mapIJKtoDX(w, x, 0, levels), mapIJKtoDY(w, x, 0, levels));
            if (w == 0)
            {
                for (int y = 1; y < 2 * x; y += 2)
                {
                    squareCount++;
                    printf("dx: %d dy: %d\n", mapIJKtoDX(w, x, y, levels), mapIJKtoDY(w, x, y, levels));
                }
            }
            else if (x > w && w != 0)
            {
            	for (int y = 1; y < 2 * w; y += 2)
                {
                    squareCount++;
                    printf("dx: %d dy: %d\n", mapIJKtoDX(w, x, y, levels), mapIJKtoDY(w, x, y, levels));
                }
            }
        }
    }

    printf("%d\n", squareCount);


    //             ___
    //         ___|___|___
    //     ___|___|___|___|___
    // ___|___|___|___|___|___|___
    //|___|___|___|___|___|___|___|
    //    |___|___|___|___|___|
    //        |___|___|___|
    //            |___|

    //Test valid

    printf("All Tests Run Successfully\n");
}