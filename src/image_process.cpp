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

#define MAXPIXIJ(w,h,x,y,i,j,levels,memStart,pPtr,memEnd) validPixel(w,h,x,y,mapIJtoDX(i,j,levels),mapIJtoDY(i,j,levels))?getPixel(w,h,x,y,mapIJtoDX(i,j,levels),mapIJtoDY(i,j,levels),memStart,pPtr,memEnd):FLOATMAX
#define MINPIXIJ(w,h,x,y,i,j,levels,memStart,pPtr,memEnd) validPixel(w,h,x,y,mapIJtoDX(i,j,levels),mapIJtoDY(i,j,levels))?getPixel(w,h,x,y,mapIJtoDX(i,j,levels),mapIJtoDY(i,j,levels),memStart,pPtr,memEnd):FLOATMIN

#include "image_process.hpp"

//MIN
void erode(unsigned w, unsigned h, int levels, uint32_t pixPerChunk, uint32_t chunksPerLine, uint64_t *chunksProcessed, uint64_t chunksRead, float *pixBufStart, float *pixCalculate, float *pixBufEnd, float *processedResultsBuffer)
{
    //Cross Shape
    if (levels == 1)
    {
        if (pixPerChunk == 4)
        {
            unsigned x;
            unsigned y;
            calculateChunkXY(w, h, &x, &y, *chunksProcessed, chunksPerLine, pixPerChunk);

            float *pix0 = pixCalculate;
            float *pix1 = pixCalculate + 1;
            float *pix2 = pixCalculate + 2;
            float *pix3 = pixCalculate + 3;

            __m128 originalPix = _mm_set_ps(*pix0, *pix1, *pix2, *pix3);

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
            _mm_store_ps(processedResultsBuffer, originalPix);

            return;
        }
        //Else we have 8, must be 1 bit packed so do it twice
        else
        {
            for (int i = 0; i < 1; i++)
            {
                unsigned x;
                unsigned y;
                calculateChunkXY(w, h, &x, &y, *chunksProcessed, chunksPerLine, pixPerChunk);

                x += 4 * i;

                float *pix0 = pixCalculate;
                float *pix1 = pixCalculate + 1;
                float *pix2 = pixCalculate + 2;
                float *pix3 = pixCalculate + 3;

                __m128 originalPix = _mm_set_ps(*pix0, *pix1, *pix2, *pix3);

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

                _mm_store_ps(processedResultsBuffer, originalPix);

                pixCalculate += 4;
                processedResultsBuffer += 4;

            }
            return;

        }
    }
    //Diamond
    else
    {
        if (pixPerChunk == 4)
        {
            unsigned x;
            unsigned y;
            calculateChunkXY(w, h, &x, &y, *chunksProcessed, chunksPerLine, pixPerChunk);

            float *pix0 = pixCalculate;
            float *pix1 = pixCalculate + 1;
            float *pix2 = pixCalculate + 2;
            float *pix3 = pixCalculate + 3;

            __m128 originalPix = _mm_set_ps(*pix0, *pix1, *pix2, *pix3);

            __m128 comparisonPix;

            for (int i = 0; i < levels + 1; i++)
            {
                for (int j = 0; j < levels + 1; j++)
                {
                    comparisonPix = _mm_set_ps(MAXPIXIJ(w, h, x, y, i, j, levels, pixBufStart, pix0, pixBufEnd), MAXPIXIJ(w, h, x + 1, y, i, j, levels, pixBufStart, pix1, pixBufEnd), MAXPIXIJ(w, h, x + 2, y, i, j, levels, pixBufStart, pix2, pixBufEnd), MAXPIXIJ(w, h, x + 3, y, i, j, levels, pixBufStart, pix3, pixBufEnd));

                    originalPix = _mm_min_ps(originalPix, comparisonPix);
                }
            }

            _mm_store_ps(processedResultsBuffer, originalPix);
        }
        else
        {
            for (int i = 0; i < 1; i++)
            {
                unsigned x;
                unsigned y;
                calculateChunkXY(w, h, &x, &y, *chunksProcessed, chunksPerLine, pixPerChunk);

                x += 4 * i;

                float *pix0 = pixCalculate;
                float *pix1 = pixCalculate + 1;
                float *pix2 = pixCalculate + 2;
                float *pix3 = pixCalculate + 3;

                __m128 originalPix = _mm_set_ps(*pix0, *pix1, *pix2, *pix3);

                __m128 comparisonPix;

                for (int i = 0; i < levels + 1; i++)
                {
                    for (int j = 0; j < levels + 1; j++)
                    {
                        comparisonPix = _mm_set_ps(MAXPIXIJ(w, h, x, y, i, j, levels, pixBufStart, pix0, pixBufEnd), MAXPIXIJ(w, h, x + 1, y, i, j, levels, pixBufStart, pix1, pixBufEnd), MAXPIXIJ(w, h, x + 2, y, i, j, levels, pixBufStart, pix2, pixBufEnd), MAXPIXIJ(w, h, x + 3, y, i, j, levels, pixBufStart, pix3, pixBufEnd));

                        originalPix = _mm_min_ps(originalPix, comparisonPix);
                    }
                }

                _mm_store_ps(processedResultsBuffer, originalPix);

                pixCalculate += 4;
                processedResultsBuffer += 4;
            }

        }

    }
}

//MAX
void dilate(unsigned w, unsigned h, int levels, uint32_t pixPerChunk, uint32_t chunksPerLine, uint64_t *chunksProcessed, uint64_t chunksRead, float *pixBufStart, float *pixCalculate, float *pixBufEnd, float *processedResultsBuffer)
{
    if (levels == 1)
    {
        if (pixPerChunk == 4)
        {
            unsigned x;
            unsigned y;
            calculateChunkXY(w, h, &x, &y, *chunksProcessed, chunksPerLine, pixPerChunk);

            float *pix0 = pixCalculate;
            float *pix1 = pixCalculate + 1;
            float *pix2 = pixCalculate + 2;
            float *pix3 = pixCalculate + 3;

            __m128 originalPix = _mm_set_ps(*pix0, *pix1, *pix2, *pix3);


            __m128 comparisonPix;

            //Above
            comparisonPix = _mm_set_ps(MINPIX(w, h, x, y, 0, -1, pixBufStart, pix0, pixBufEnd), MINPIX(w, h, x + 1, y, 0, -1, pixBufStart, pix1, pixBufEnd), MINPIX(w, h, x + 2, y, 0, -1, pixBufStart, pix2, pixBufEnd), MINPIX(w, h, x + 3, y, 0, -1, pixBufStart, pix3, pixBufEnd));

            originalPix = _mm_min_ps(originalPix, comparisonPix);

            //Below
            comparisonPix = _mm_set_ps(MINPIX(w, h, x, y, 0, 1, pixBufStart, pix0, pixBufEnd), MINPIX(w, h, x + 1, y, 0, 1, pixBufStart, pix1, pixBufEnd), MINPIX(w, h, x + 2, y, 0, 1, pixBufStart, pix2, pixBufEnd), MINPIX(w, h, x + 3, y, 0, 1, pixBufStart, pix3, pixBufEnd));

            originalPix = _mm_min_ps(originalPix, comparisonPix);

            //Left
            comparisonPix = _mm_set_ps(MINPIX(w, h, x, y, -1, 0, pixBufStart, pix0, pixBufEnd), MINPIX(w, h, x + 1, y, -1, 0, pixBufStart, pix1, pixBufEnd), MINPIX(w, h, x + 2, y, -1, 0, pixBufStart, pix2, pixBufEnd), MINPIX(w, h, x + 3, y, -1, 0, pixBufStart, pix3, pixBufEnd));

            originalPix = _mm_min_ps(originalPix, comparisonPix);

            //Right
            comparisonPix = _mm_set_ps(MINPIX(w, h, x, y, 1, 0, pixBufStart, pix0, pixBufEnd), MINPIX(w, h, x + 1, y, 1, 0, pixBufStart, pix1, pixBufEnd), MINPIX(w, h, x + 2, y, 1, 0, pixBufStart, pix2, pixBufEnd), MINPIX(w, h, x + 3, y, 1, 0, pixBufStart, pix3, pixBufEnd));

            originalPix = _mm_min_ps(originalPix, comparisonPix);

            //Store the result
            _mm_store_ps(processedResultsBuffer, originalPix);

            return;
        }
        //Else we have 8, must be 1 bit packed
        else
        {
            for (int i = 0; i < 1; i++)
            {
                unsigned x;
                unsigned y;
                calculateChunkXY(w, h, &x, &y, *chunksProcessed, chunksPerLine, pixPerChunk);

                x += 4 * i;

                float *pix0 = pixCalculate;
                float *pix1 = pixCalculate + 1;
                float *pix2 = pixCalculate + 2;
                float *pix3 = pixCalculate + 3;

                __m128 originalPix = _mm_set_ps(*pix0, *pix1, *pix2, *pix3);


                __m128 comparisonPix;

                //Above
                comparisonPix = _mm_set_ps(MINPIX(w, h, x, y, 0, -1, pixBufStart, pix0, pixBufEnd), MINPIX(w, h, x + 1, y, 0, -1, pixBufStart, pix1, pixBufEnd), MINPIX(w, h, x + 2, y, 0, -1, pixBufStart, pix2, pixBufEnd), MINPIX(w, h, x + 3, y, 0, -1, pixBufStart, pix3, pixBufEnd));

                originalPix = _mm_min_ps(originalPix, comparisonPix);

                //Below
                comparisonPix = _mm_set_ps(MINPIX(w, h, x, y, 0, 1, pixBufStart, pix0, pixBufEnd), MINPIX(w, h, x + 1, y, 0, 1, pixBufStart, pix1, pixBufEnd), MINPIX(w, h, x + 2, y, 0, 1, pixBufStart, pix2, pixBufEnd), MINPIX(w, h, x + 3, y, 0, 1, pixBufStart, pix3, pixBufEnd));

                originalPix = _mm_min_ps(originalPix, comparisonPix);

                //Left
                comparisonPix = _mm_set_ps(MINPIX(w, h, x, y, -1, 0, pixBufStart, pix0, pixBufEnd), MINPIX(w, h, x + 1, y, -1, 0, pixBufStart, pix1, pixBufEnd), MINPIX(w, h, x + 2, y, -1, 0, pixBufStart, pix2, pixBufEnd), MINPIX(w, h, x + 3, y, -1, 0, pixBufStart, pix3, pixBufEnd));

                originalPix = _mm_min_ps(originalPix, comparisonPix);

                //Right
                comparisonPix = _mm_set_ps(MINPIX(w, h, x, y, 1, 0, pixBufStart, pix0, pixBufEnd), MINPIX(w, h, x + 1, y, 1, 0, pixBufStart, pix1, pixBufEnd), MINPIX(w, h, x + 2, y, 1, 0, pixBufStart, pix2, pixBufEnd), MINPIX(w, h, x + 3, y, 1, 0, pixBufStart, pix3, pixBufEnd));

                originalPix = _mm_min_ps(originalPix, comparisonPix);

                //Store the result
                _mm_store_ps(processedResultsBuffer, originalPix);

                pixCalculate += 4;
                processedResultsBuffer += 4;

            }
            return;
        }
    }
    //Diamond
    else
    {
        if (pixPerChunk == 4)
        {
            unsigned x;
            unsigned y;
            calculateChunkXY(w, h, &x, &y, *chunksProcessed, chunksPerLine, pixPerChunk);

            float *pix0 = pixCalculate;
            float *pix1 = pixCalculate + 1;
            float *pix2 = pixCalculate + 2;
            float *pix3 = pixCalculate + 3;

            __m128 originalPix = _mm_set_ps(*pix0, *pix1, *pix2, *pix3);

            __m128 comparisonPix;

            for (int i = 0; i < levels + 1; i++)
            {
                for (int j = 0; j < levels + 1; j++)
                {
                    comparisonPix = _mm_set_ps(MINPIXIJ(w, h, x, y, i, j, levels, pixBufStart, pix0, pixBufEnd), MINPIXIJ(w, h, x + 1, y, i, j, levels, pixBufStart, pix1, pixBufEnd), MINPIXIJ(w, h, x + 2, y, i, j, levels, pixBufStart, pix2, pixBufEnd), MINPIXIJ(w, h, x + 3, y, i, j, levels, pixBufStart, pix3, pixBufEnd));

                    originalPix = _mm_min_ps(originalPix, comparisonPix);
                }
            }

            _mm_store_ps(processedResultsBuffer, originalPix);

            return;
        }
        else
        {
            for (int i = 0; i < 1; i++)
            {
                unsigned x;
                unsigned y;
                calculateChunkXY(w, h, &x, &y, *chunksProcessed, chunksPerLine, pixPerChunk);

                x += 4 * i;

                float *pix0 = pixCalculate;
                float *pix1 = pixCalculate + 1;
                float *pix2 = pixCalculate + 2;
                float *pix3 = pixCalculate + 3;

                __m128 originalPix = _mm_set_ps(*pix0, *pix1, *pix2, *pix3);

                __m128 comparisonPix;

                for (int i = 0; i < levels + 1; i++)
                {
                    for (int j = 0; j < levels + 1; j++)
                    {
                        comparisonPix = _mm_set_ps(MINPIXIJ(w, h, x, y, i, j, levels, pixBufStart, pix0, pixBufEnd), MINPIXIJ(w, h, x + 1, y, i, j, levels, pixBufStart, pix1, pixBufEnd), MINPIXIJ(w, h, x + 2, y, i, j, levels, pixBufStart, pix2, pixBufEnd), MINPIXIJ(w, h, x + 3, y, i, j, levels, pixBufStart, pix3, pixBufEnd));

                        originalPix = _mm_min_ps(originalPix, comparisonPix);
                    }
                }

                _mm_store_ps(processedResultsBuffer, originalPix);

                pixCalculate += 4;
                processedResultsBuffer += 4;
            }

            return;

        }

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
    float *result = pixPtr + dx + w * dy;

    if (result < memStart)
    {
        result = memEnd - (memStart - result);
    }
    else if (result >= memEnd)
    {
        result = memStart + (result - memEnd);
    }

    return *result;

}

int mapIJtoDX(int i, int j, int levels)
{
    return j - i;
}

int mapIJtoDY(int i, int j, int levels)
{
    return -levels + i + j;
}

int mapIJKtoDX(int i, int j, int k, int levels)
{
    return j - i - k;
}

int mapIJKtoDY(int i, int j, int k, int levels)
{
    return -levels + i + j;
}

void mapIJtodXdY(int i, int j, int levels, int *dX, int *dY)
{
    *dX = j - i;
    *dY = -levels + i + j;
}

void calculateChunkXY(unsigned w, unsigned h, unsigned *x, unsigned *y, uint64_t chunksProcessed, uint32_t chunksPerLine, uint32_t pixPerChunk)
{
    *x = chunksProcessed % chunksPerLine * pixPerChunk;
    *y = chunksProcessed / chunksPerLine;
}

void processStreamChunk(unsigned w, unsigned h, int levels, uint32_t pixPerChunk, uint32_t chunksPerLine, uint64_t *chunksProcessed, uint64_t chunksRead, float *pixBufStart, float **pixCalculate, float *pixBufEnd, float *processedResultsBuffer)
{
    switch (levels > 0)
    {
    case true:
        // dilate();
        std::memcpy(*pixCalculate, processedResultsBuffer, sizeof(float) * pixPerChunk);
        // erode();
        break;
    case false:
        // erode();
        std::memcpy(*pixCalculate, processedResultsBuffer, sizeof(float) * pixPerChunk);
        // dilate();
        break;
    }

    (*pixCalculate) += pixPerChunk;
    if (*pixCalculate == pixBufEnd)
    {
        (*pixCalculate) = pixBufStart;
    }
}