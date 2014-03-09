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

#include <limits> //Include max and min for float

#include "xmmintrin.h" // Intrinsic Functions!

#define FLOATMIN std::numeric_limits<float>::min()
#define FLOATMAX std::numeric_limits<float>::max()

#define MAXPIX(w,h,x,y,dx,dy,memStart,pPtr,memEnd) validPixel(w,h,x,y,dx,dy)?getPixel(w,h,x,y,dx,dy,memStart,pPtr,memEnd):FLOATMAX

#define MINPIX(w,h,x,y,dx,dy,memStart,pPtr,memEnd) validPixel(w,h,x,y,dx,dy)?getPixel(w,h,x,y,dx,dy,memStart,pPtr,memEnd):FLOATMIN

#define MAXPIXIJK(w,h,x,y,i,j,k,levels,memStart,pPtr,memEnd) validPixel(w,h,x,y,mapIJKtoDX(i,j,k,levels),mapIJKtoDY(i,j,k,levels))?getPixel(w,h,x,y,mapIJKtoDX(i,j,k,levels),mapIJKtoDY(i,j,k,levels),memStart,pPtr,memEnd):FLOATMAX

#define MINPIXIJK(w,h,x,y,i,j,k,levels,memStart,pPtr,memEnd) validPixel(w,h,x,y,mapIJKtoDX(i,j,k,levels),mapIJKtoDY(i,j,k,levels))?getPixel(w,h,x,y,mapIJKtoDX(i,j,k,levels),mapIJKtoDY(i,j,k,levels),memStart,pPtr,memEnd):FLOATMIN

#include "image_process.hpp"

//MIN
void erodeChunk(unsigned w, unsigned h, int levels, uint32_t pixPerChunk, uint32_t chunksPerLine, uint64_t *chunksProcessed, float *pixBufStart, float *pixCalculate, float *pixBufEnd, float *processedResultsBuffer)
{
    //Cross Shape
    if (levels == 1)
    {
        for (int i = 0; i < pixPerChunk / 4; i++)
        {
            unsigned x;
            unsigned y;
            calculateChunkXY(w, h, &x, &y, *chunksProcessed, chunksPerLine, pixPerChunk);

            x += 4 * i;

            float *pix0 = pixCalculate;
            float *pix1 = pixCalculate + 1;
            float *pix2 = pixCalculate + 2;
            float *pix3 = pixCalculate + 3;

            __m128 originalPix;

            originalPix = _mm_set_ps(*pix0, *pix1, *pix2, *pix3);

            __m128 comparisonPix;

            //Above
            comparisonPix = _mm_set_ps(MAXPIX(w, h, x, y, 0, -1, pixBufStart, pix0, pixBufEnd), MAXPIX(w, h, x + 1, y, 0, -1, pixBufStart, pix1, pixBufEnd), MAXPIX(w, h, x + 2, y, 0, -1, pixBufStart, pix2, pixBufEnd), MAXPIX(w, h, x + 3, y, 0, -1, pixBufStart, pix3, pixBufEnd));

            originalPix = _mm_min_ps(originalPix, comparisonPix);

            //Below
            comparisonPix = _mm_set_ps(MAXPIX(w, h, x, y, 0, 1, pixBufStart, pix0, pixBufEnd), MAXPIX(w, h, x + 1, y, 0, 1, pixBufStart, pix1, pixBufEnd), MAXPIX(w, h, x + 2, y, 0, 1, pixBufStart, pix2, pixBufEnd), MAXPIX(w, h, x + 3, y, 0, 1, pixBufStart, pix3, pixBufEnd));

            originalPix = _mm_min_ps(originalPix, comparisonPix);

            //Left
            comparisonPix = _mm_set_ps(MAXPIX(w, h, x, y, -1, 0, pixBufStart, pix0, pixBufEnd), MAXPIX(w, h, x + 1, y, -1, 0, pixBufStart, pix1, pixBufEnd), MAXPIX(w, h, x + 2, y, -1, 0, pixBufStart, pix2, pixBufEnd), MAXPIX(w, h, x + 3, y, -1, 0, pixBufStart, pix3, pixBufEnd));

            originalPix = _mm_min_ps(originalPix, comparisonPix);

            //Right
            comparisonPix = _mm_set_ps(MAXPIX(w, h, x, y, 1, 0, pixBufStart, pix0, pixBufEnd), MAXPIX(w, h, x + 1, y, 1, 0, pixBufStart, pix1, pixBufEnd), MAXPIX(w, h, x + 2, y, 1, 0, pixBufStart, pix2, pixBufEnd), MAXPIX(w, h, x + 3, y, 1, 0, pixBufStart, pix3, pixBufEnd));

            originalPix = _mm_min_ps(originalPix, comparisonPix);

            //Store the result

            float temp[4];
            _mm_storer_ps(&temp[0], originalPix);
            std::memcpy(processedResultsBuffer, &temp[0], sizeof(float) * 4);

            pixCalculate += 4;
            processedResultsBuffer += 4;

        }

        (*chunksProcessed)++;
        return;

    }
    //Diamond
    else
    {
        for (int i = 0; i < pixPerChunk / 4; i++)
        {
            unsigned x;
            unsigned y;
            calculateChunkXY(w, h, &x, &y, *chunksProcessed, chunksPerLine, pixPerChunk);

            x += 4 * i;

            float *pix0 = pixCalculate;
            float *pix1 = pixCalculate + 1;
            float *pix2 = pixCalculate + 2;
            float *pix3 = pixCalculate + 3;

            __m128 originalPix;

            originalPix = _mm_set_ps(*pix0, *pix1, *pix2, *pix3);

            __m128 comparisonPix;

            for (int i = 0; i < levels + 1; i++)
            {
                for (int j = 0; j < levels + 1; j++)
                {
                    comparisonPix = _mm_set_ps(MAXPIXIJK(w, h, x, y, i, j, 0, levels, pixBufStart, pix0, pixBufEnd), MAXPIXIJK(w, h, x + 1, y, i, j, 0, levels, pixBufStart, pix1, pixBufEnd), MAXPIXIJK(w, h, x + 2, y, i, j, 0, levels, pixBufStart, pix2, pixBufEnd), MAXPIXIJK(w, h, x + 3, y, i, j, 0, levels, pixBufStart, pix3, pixBufEnd));

                    originalPix = _mm_min_ps(originalPix, comparisonPix);

                    if (i == 0)
                    {
                        for (int k = 1; k < 2 * j; k += 2)
                        {
                            comparisonPix = _mm_set_ps(MAXPIXIJK(w, h, x, y, i, j, k, levels, pixBufStart, pix0, pixBufEnd), MAXPIXIJK(w, h, x + 1, y, i, j, k, levels, pixBufStart, pix1, pixBufEnd), MAXPIXIJK(w, h, x + 2, y, i, j, k, levels, pixBufStart, pix2, pixBufEnd), MAXPIXIJK(w, h, x + 3, y, i, j, k, levels, pixBufStart, pix3, pixBufEnd));

                            originalPix = _mm_min_ps(originalPix, comparisonPix);

                        }
                    }
                    else if (i < j && i != 0 && j == levels)
                    {
                        for (int k = 1; k < 2 * (levels - i); k += 2)
                        {
                            comparisonPix = _mm_set_ps(MAXPIXIJK(w, h, x, y, i, j, k, levels, pixBufStart, pix0, pixBufEnd), MAXPIXIJK(w, h, x + 1, y, i, j, k, levels, pixBufStart, pix1, pixBufEnd), MAXPIXIJK(w, h, x + 2, y, i, j, k, levels, pixBufStart, pix2, pixBufEnd), MAXPIXIJK(w, h, x + 3, y, i, j, k, levels, pixBufStart, pix3, pixBufEnd));

                            originalPix = _mm_min_ps(originalPix, comparisonPix);
                        }
                    }
                }
            }

            float temp[4];
            _mm_storer_ps(&temp[0], originalPix);
            std::memcpy(processedResultsBuffer, &temp[0], sizeof(float) * 4);

            pixCalculate += 4;
            processedResultsBuffer += 4;
        }

        (*chunksProcessed)++;
        return;

    }

}

//MAX
void dilateChunk(unsigned w, unsigned h, int levels, uint32_t pixPerChunk, uint32_t chunksPerLine, uint64_t *chunksProcessed, float *pixBufStart, float *pixCalculate, float *pixBufEnd, float *processedResultsBuffer)
{
    if (levels == 1)
    {
        for (int i = 0; i < pixPerChunk / 4; i++)
        {
            unsigned x;
            unsigned y;
            calculateChunkXY(w, h, &x, &y, *chunksProcessed, chunksPerLine, pixPerChunk);

            x += 4 * i;

            float *pix0 = pixCalculate;
            float *pix1 = pixCalculate + 1;
            float *pix2 = pixCalculate + 2;
            float *pix3 = pixCalculate + 3;

            __m128 originalPix;

            originalPix = _mm_set_ps(*pix0, *pix1, *pix2, *pix3);

            __m128 comparisonPix;

            //Above
            comparisonPix = _mm_set_ps(MINPIX(w, h, x, y, 0, -1, pixBufStart, pix0, pixBufEnd), MINPIX(w, h, x + 1, y, 0, -1, pixBufStart, pix1, pixBufEnd), MINPIX(w, h, x + 2, y, 0, -1, pixBufStart, pix2, pixBufEnd), MINPIX(w, h, x + 3, y, 0, -1, pixBufStart, pix3, pixBufEnd));

            originalPix = _mm_max_ps(originalPix, comparisonPix);

            //Below
            comparisonPix = _mm_set_ps(MINPIX(w, h, x, y, 0, 1, pixBufStart, pix0, pixBufEnd), MINPIX(w, h, x + 1, y, 0, 1, pixBufStart, pix1, pixBufEnd), MINPIX(w, h, x + 2, y, 0, 1, pixBufStart, pix2, pixBufEnd), MINPIX(w, h, x + 3, y, 0, 1, pixBufStart, pix3, pixBufEnd));

            originalPix = _mm_max_ps(originalPix, comparisonPix);

            //Left
            comparisonPix = _mm_set_ps(MINPIX(w, h, x, y, -1, 0, pixBufStart, pix0, pixBufEnd), MINPIX(w, h, x + 1, y, -1, 0, pixBufStart, pix1, pixBufEnd), MINPIX(w, h, x + 2, y, -1, 0, pixBufStart, pix2, pixBufEnd), MINPIX(w, h, x + 3, y, -1, 0, pixBufStart, pix3, pixBufEnd));

            originalPix = _mm_max_ps(originalPix, comparisonPix);

            //Right
            comparisonPix = _mm_set_ps(MINPIX(w, h, x, y, 1, 0, pixBufStart, pix0, pixBufEnd), MINPIX(w, h, x + 1, y, 1, 0, pixBufStart, pix1, pixBufEnd), MINPIX(w, h, x + 2, y, 1, 0, pixBufStart, pix2, pixBufEnd), MINPIX(w, h, x + 3, y, 1, 0, pixBufStart, pix3, pixBufEnd));

            originalPix = _mm_max_ps(originalPix, comparisonPix);

            //Store the result
            float temp[4];
            _mm_storer_ps(&temp[0], originalPix);
            std::memcpy(processedResultsBuffer, &temp[0], sizeof(float) * 4);

            pixCalculate += 4;
            processedResultsBuffer += 4;

        }

        (*chunksProcessed)++;
        return;

    }
    //Diamond
    else
    {
        for (int i = 0; i < pixPerChunk / 4; i++)
        {
            unsigned x;
            unsigned y;
            calculateChunkXY(w, h, &x, &y, *chunksProcessed, chunksPerLine, pixPerChunk);

            x += 4 * i;

            float *pix0 = pixCalculate;
            float *pix1 = pixCalculate + 1;
            float *pix2 = pixCalculate + 2;
            float *pix3 = pixCalculate + 3;

            __m128 originalPix;

            originalPix = _mm_set_ps(*pix0, *pix1, *pix2, *pix3);

            __m128 comparisonPix;

            for (int i = 0; i < levels + 1; i++)
            {
                for (int j = 0; j < levels + 1; j++)
                {
                    comparisonPix = _mm_set_ps(MINPIXIJK(w, h, x, y, i, j, 0, levels, pixBufStart, pix0, pixBufEnd), MINPIXIJK(w, h, x + 1, y, i, j, 0, levels, pixBufStart, pix1, pixBufEnd), MINPIXIJK(w, h, x + 2, y, i, j, 0, levels, pixBufStart, pix2, pixBufEnd), MINPIXIJK(w, h, x + 3, y, i, j, 0, levels, pixBufStart, pix3, pixBufEnd));

                    originalPix = _mm_max_ps(originalPix, comparisonPix);

                    if (i == 0)
                    {
                        for (int k = 1; k < 2 * j; k += 2)
                        {
                            comparisonPix = _mm_set_ps(MINPIXIJK(w, h, x, y, i, j, k, levels, pixBufStart, pix0, pixBufEnd), MINPIXIJK(w, h, x + 1, y, i, j, k, levels, pixBufStart, pix1, pixBufEnd), MINPIXIJK(w, h, x + 2, y, i, j, k, levels, pixBufStart, pix2, pixBufEnd), MINPIXIJK(w, h, x + 3, y, i, j, k, levels, pixBufStart, pix3, pixBufEnd));

                            originalPix = _mm_max_ps(originalPix, comparisonPix);

                        }
                    }
                    else if (i < j && i != 0 && j == levels)
                    {
                        for (int k = 1; k < 2 * (levels - i); k += 2)
                        {
                            comparisonPix = _mm_set_ps(MINPIXIJK(w, h, x, y, i, j, k, levels, pixBufStart, pix0, pixBufEnd), MINPIXIJK(w, h, x + 1, y, i, j, k, levels, pixBufStart, pix1, pixBufEnd), MINPIXIJK(w, h, x + 2, y, i, j, k, levels, pixBufStart, pix2, pixBufEnd), MINPIXIJK(w, h, x + 3, y, i, j, k, levels, pixBufStart, pix3, pixBufEnd));

                            originalPix = _mm_max_ps(originalPix, comparisonPix);
                        }
                    }
                }
            }

            float temp[4];
            _mm_storer_ps(&temp[0], originalPix);
            std::memcpy(processedResultsBuffer, &temp[0], sizeof(float) * 4);

            pixCalculate += 4;
            processedResultsBuffer += 4;
        }

        (*chunksProcessed)++;
        return;

    }

}

//Check if pixel to compare is in image
bool validPixel(unsigned w, unsigned h, unsigned x, unsigned y, int dx, int dy)
{
    if (x + dx < 0 || x + dx >= w)
    {
        return false;
    }
    else if (y + dy < 0 || y + dy >= h)
    {
        return false;
    }
    else
    {
        return true;
    }
}

float getPixel(unsigned w, unsigned h, unsigned x, unsigned y, int dx, int dy, float *memStart, float *pixPtr, float *memEnd)
{

	// fprintf(stderr, "Getting x %d y %d dx %d dy %d pixPtr %d\n", x, y, dx, dy, pixPtr);

    int offset = dx + (w * dy);

    // fprintf(stderr, "offset %d\n", offset);

    float *result = pixPtr + offset;

    // fprintf(stderr, "result %d memEnd %d\n", result, memEnd);

    if (result < memStart)
    {
        result = result + ((memEnd - memStart));
    }
    else if (result >= memEnd)
    {
        result = memStart + ((result - memEnd));
    }

    // fprintf(stderr, "start %d res %d end %d\n", memStart, result, memEnd);

    return *result;

}

int mapIJKtoDX(int i, int j, int k, int levels)
{
    return j - i - k;
}

int mapIJKtoDY(int i, int j, int k, int levels)
{
    return -levels + i + j;
}

void calculateChunkXY(unsigned w, unsigned h, unsigned *x, unsigned *y, uint64_t chunksProcessed, uint32_t chunksPerLine, uint32_t pixPerChunk)
{
    *x = (chunksProcessed % chunksPerLine) * pixPerChunk;
    *y = (unsigned)(chunksProcessed / chunksPerLine);
}

bool processStreamChunk(unsigned w, unsigned h, int levels, uint32_t pixPerChunk, uint32_t chunksPerLine, uint32_t totalChunks, uint64_t *originalImgChunksProcessed, float *pixBufStart, float **pixCalculate, float *pixBufEnd, uint64_t *irChunksProcessed, float *irStart, float **irInsert, float **irCalculate, float *irEnd, float *finalResult)
{

    // fprintf(stderr, "original %d irChunks %d totalChunks %d\n", *originalImgChunksProcessed, *irChunksProcessed, totalChunks);
    if (levels > 0)
    {
        if (*originalImgChunksProcessed < (totalChunks))
        {
            dilateChunk(w, h, levels, pixPerChunk, chunksPerLine, originalImgChunksProcessed, pixBufStart, *pixCalculate, pixBufEnd, *irInsert);
            // fprintf(stderr, "After Dilate %d %d %d %d %d %d %d\n", (int)*(*irInsert), (int)*(*irInsert + 1), (int)*(*irInsert + 2), (int)*(*irInsert + 3), (int)*(*irInsert + 4), (int)*(*irInsert + 5), (int)*(*irInsert + 6), (int)*(*irInsert + 7));
            *pixCalculate += pixPerChunk;
            if (*pixCalculate >= pixBufEnd)
            {
                *pixCalculate = pixBufStart + ((*pixCalculate - pixBufEnd) / sizeof(float));
            }
            *(irInsert) += pixPerChunk;
            if (*irInsert >= irEnd)
            {
                *irInsert = irStart + ((*irInsert - irEnd) / sizeof(float));
            }
        }
        if (*originalImgChunksProcessed > (levels * chunksPerLine) && *irChunksProcessed < (totalChunks))
        {
            erodeChunk(w, h, levels, pixPerChunk, chunksPerLine, irChunksProcessed, irStart, *irCalculate, irEnd, finalResult);
            // fprintf(stderr, "After Erode %f %f %f %f\n", finalResult[0], finalResult[1], finalResult[2], finalResult[3]);
            if (*irCalculate >= irEnd)
            {
                *irCalculate = irStart + ((*irCalculate - irEnd) / sizeof(float));
            }
            return true;
        }
        return false;
    }
    else
    {
        if (*originalImgChunksProcessed < (totalChunks))
        {
            erodeChunk(w, h, levels, pixPerChunk, chunksPerLine, originalImgChunksProcessed, pixBufStart, *pixCalculate, pixBufEnd, *irInsert);
            if (*pixCalculate >= pixBufEnd)
            {
                *pixCalculate = pixBufStart + ((*pixCalculate - pixBufEnd) / sizeof(float));
            }
            *irInsert += pixPerChunk;
            if (*irInsert >= irEnd)
            {
                *irInsert = irStart + ((*irInsert - irEnd) / sizeof(float));
            }
        }
        if (*originalImgChunksProcessed > (levels * chunksPerLine))
        {
            dilateChunk(w, h, levels, pixPerChunk, chunksPerLine, irChunksProcessed, irStart, *irCalculate, irEnd, finalResult);
            *irCalculate += pixPerChunk;
            if (*irCalculate >= irEnd)
            {
                *irCalculate = irStart + ((*irCalculate - irEnd) / sizeof(float));
            }
            return true;
        }
        return false;
    }
}
