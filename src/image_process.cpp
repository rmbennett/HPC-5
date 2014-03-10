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

//Define macro to get minimum/maximum representable value of float
#define FLOATMIN std::numeric_limits<float>::min()
#define FLOATMAX std::numeric_limits<float>::max()

//Return pixel value at desired offset or max value for float if pixel being fetched is "outside" the image
#define MAXPIX(w,h,x,y,dx,dy,memStart,pPtr,memEnd) validPixel(w,h,x,y,dx,dy)?getPixel(w,h,x,y,dx,dy,memStart,pPtr,memEnd):FLOATMAX

//Return pixel value at desired offset or min value for float if pixel being fetched is "outside" the image
#define MINPIX(w,h,x,y,dx,dy,memStart,pPtr,memEnd) validPixel(w,h,x,y,dx,dy)?getPixel(w,h,x,y,dx,dy,memStart,pPtr,memEnd):FLOATMIN

//Return pixel value at offset calculated by IJK offet or max value for float if pixel being fetched is "outside" the image
#define MAXPIXIJK(w,h,x,y,i,j,k,levels,memStart,pPtr,memEnd) validPixel(w,h,x,y,mapIJKtoDX(i,j,k,levels),mapIJKtoDY(i,j,k,levels))?getPixel(w,h,x,y,mapIJKtoDX(i,j,k,levels),mapIJKtoDY(i,j,k,levels),memStart,pPtr,memEnd):FLOATMAX

//Return pixel value at offset calculated by IJK offet or min value for float if pixel being fetched is "outside" the image
#define MINPIXIJK(w,h,x,y,i,j,k,levels,memStart,pPtr,memEnd) validPixel(w,h,x,y,mapIJKtoDX(i,j,k,levels),mapIJKtoDY(i,j,k,levels))?getPixel(w,h,x,y,mapIJKtoDX(i,j,k,levels),mapIJKtoDY(i,j,k,levels),memStart,pPtr,memEnd):FLOATMIN

//Define macro to get minimum/maximum representable value of double
#define DOUBLEMIN std::numeric_limits<double>::min()
#define DOUBLEMAX std::numeric_limits<double>::max()

//Same as above but with doubles for case where bits=32
#define MAXPIX32(w,h,x,y,dx,dy,memStart,pPtr,memEnd) validPixel(w,h,x,y,dx,dy)?getPixel32(w,h,x,y,dx,dy,memStart,pPtr,memEnd):DOUBLEMAX

#define MINPIX32(w,h,x,y,dx,dy,memStart,pPtr,memEnd) validPixel(w,h,x,y,dx,dy)?getPixel32(w,h,x,y,dx,dy,memStart,pPtr,memEnd):DOUBLEMIN

#define MAXPIXIJK32(w,h,x,y,i,j,k,levels,memStart,pPtr,memEnd) validPixel(w,h,x,y,mapIJKtoDX(i,j,k,levels),mapIJKtoDY(i,j,k,levels))?getPixel32(w,h,x,y,mapIJKtoDX(i,j,k,levels),mapIJKtoDY(i,j,k,levels),memStart,pPtr,memEnd):DOUBLEMAX

#define MINPIXIJK32(w,h,x,y,i,j,k,levels,memStart,pPtr,memEnd) validPixel(w,h,x,y,mapIJKtoDX(i,j,k,levels),mapIJKtoDY(i,j,k,levels))?getPixel32(w,h,x,y,mapIJKtoDX(i,j,k,levels),mapIJKtoDY(i,j,k,levels),memStart,pPtr,memEnd):DOUBLEMIN

#include "image_process.hpp"

//Min of neighbouring pixel neighbourhood
void erodeChunk(unsigned w, unsigned h, int levels, uint32_t pixPerChunk, uint32_t chunksPerLine, uint64_t *chunksProcessed, float *pixBufStart, float *pixCalculate, float *pixBufEnd, float *processedResultsBuffer)
{
    unsigned x;
    unsigned y;
    //Get X Y coordinate of this chunk
    calculateChunkXY(w, h, &x, &y, *chunksProcessed, chunksPerLine, pixPerChunk);

    //Cross Shape
    if (levels == 1)
    {
        for (int i = 0; i < pixPerChunk / 4; i++)
        {
            //Get four sequential pixels from block
            float *pix0 = &pixCalculate[0];
            float *pix1 = &pixCalculate[1];
            float *pix2 = &pixCalculate[2];
            float *pix3 = &pixCalculate[3];

            //Load them into a 128 bit vector of floats
            __m128 originalPix = _mm_set_ps(*pix0, *pix1, *pix2, *pix3);

            __m128 comparisonPix;

            //Above
            comparisonPix = _mm_set_ps(
                                MAXPIX(w, h, x, y, 0, -1, pixBufStart, pix0, pixBufEnd),
                                MAXPIX(w, h, x + 1, y, 0, -1, pixBufStart, pix1, pixBufEnd),
                                MAXPIX(w, h, x + 2, y, 0, -1, pixBufStart, pix2, pixBufEnd),
                                MAXPIX(w, h, x + 3, y, 0, -1, pixBufStart, pix3, pixBufEnd));

            //Get the min and store into originalPix
            originalPix = _mm_min_ps(originalPix, comparisonPix);

            //Below
            comparisonPix = _mm_set_ps(
                                MAXPIX(w, h, x, y, 0, 1, pixBufStart, pix0, pixBufEnd),
                                MAXPIX(w, h, x + 1, y, 0, 1, pixBufStart, pix1, pixBufEnd),
                                MAXPIX(w, h, x + 2, y, 0, 1, pixBufStart, pix2, pixBufEnd),
                                MAXPIX(w, h, x + 3, y, 0, 1, pixBufStart, pix3, pixBufEnd));

            originalPix = _mm_min_ps(originalPix, comparisonPix);

            //Left
            comparisonPix = _mm_set_ps(
                                MAXPIX(w, h, x, y, -1, 0, pixBufStart, pix0, pixBufEnd),
                                MAXPIX(w, h, x + 1, y, -1, 0, pixBufStart, pix1, pixBufEnd),
                                MAXPIX(w, h, x + 2, y, -1, 0, pixBufStart, pix2, pixBufEnd),
                                MAXPIX(w, h, x + 3, y, -1, 0, pixBufStart, pix3, pixBufEnd));

            originalPix = _mm_min_ps(originalPix, comparisonPix);

            //Right
            comparisonPix = _mm_set_ps(
                                MAXPIX(w, h, x, y, 1, 0, pixBufStart, pix0, pixBufEnd),
                                MAXPIX(w, h, x + 1, y, 1, 0, pixBufStart, pix1, pixBufEnd),
                                MAXPIX(w, h, x + 2, y, 1, 0, pixBufStart, pix2, pixBufEnd),
                                MAXPIX(w, h, x + 3, y, 1, 0, pixBufStart, pix3, pixBufEnd));

            originalPix = _mm_min_ps(originalPix, comparisonPix);

            //Store the result into result buffer
            _mm_storer_ps(processedResultsBuffer, originalPix);

            //Increment x by number of pixels processed
            x += 4;
            //Increment pointer by number of pixels processed
            pixCalculate += 4;
            //Increment results buffer pointer by number of pixels written
            processedResultsBuffer += 4;

        }

        //Increment number of chunks processed
        (*chunksProcessed)++;
        return;

    }
    //Diamond
    else
    {
        for (int i = 0; i < pixPerChunk / 4; i++)
        {
            float *pix0 = &pixCalculate[0];
            float *pix1 = &pixCalculate[1];
            float *pix2 = &pixCalculate[2];
            float *pix3 = &pixCalculate[3];

            __m128 originalPix = _mm_set_ps(*pix0, *pix1, *pix2, *pix3);

            __m128 comparisonPix;

            //Increment i down left upper diagonal edge
            for (int i = 0; i < levels + 1; i++)
            {
                //Iterate j down right upper diagonal edge
                for (int j = 0; j < levels + 1; j++)
                {
                    //Do that pixel, gives the following

                    //          ___
                    //     i___|0,0|___ j
                    //  ___|1,0|___|0,1|___
                    // |2,0|___|1,1|___|0,2|
                    //     |2,1|___|1,2|
                    //         |2,2|

                    comparisonPix = _mm_set_ps(
                                        MAXPIXIJK(w, h, x, y, i, j, 0, levels, pixBufStart, pix0, pixBufEnd),
                                        MAXPIXIJK(w, h, x + 1, y, i, j, 0, levels, pixBufStart, pix1, pixBufEnd),
                                        MAXPIXIJK(w, h, x + 2, y, i, j, 0, levels, pixBufStart, pix2, pixBufEnd),
                                        MAXPIXIJK(w, h, x + 3, y, i, j, 0, levels, pixBufStart, pix3, pixBufEnd));

                    originalPix = _mm_min_ps(originalPix, comparisonPix);

                    //If we're on the top right diagonal edge
                    if (i == 0)
                    {
                        //Work backwards and catch and elements we'd otherwise miss, marked with X's

                        //          ___
                        //     i___|0,0|___ j
                        //  ___|1,0|_X_|0,1|___
                        // |2,0|_X_|1,1|_X_|0,2|
                        //     |2,1|___|1,2|
                        //         |2,2|

                        for (int k = 1; k < 2 * j; k += 2)
                        {
                            comparisonPix = _mm_set_ps(
                                                MAXPIXIJK(w, h, x, y, i, j, k, levels, pixBufStart, pix0, pixBufEnd),
                                                MAXPIXIJK(w, h, x + 1, y, i, j, k, levels, pixBufStart, pix1, pixBufEnd),
                                                MAXPIXIJK(w, h, x + 2, y, i, j, k, levels, pixBufStart, pix2, pixBufEnd),
                                                MAXPIXIJK(w, h, x + 3, y, i, j, k, levels, pixBufStart, pix3, pixBufEnd));

                            originalPix = _mm_min_ps(originalPix, comparisonPix);

                        }
                    }
                    //Else if we're on the bottom right diagonal edge, do the same, marked with Y's

                    //          ___
                    //     i___|0,0|___ j
                    //  ___|1,0|_X_|0,1|___
                    // |2,0|_X_|1,1|_X_|0,2|
                    //     |2,1|_Y_|1,2|
                    //         |2,2|

                    else if (i < j && i != 0 && j == levels)
                    {
                        for (int k = 1; k < 2 * (levels - i); k += 2)
                        {
                            comparisonPix = _mm_set_ps(
                                                MAXPIXIJK(w, h, x, y, i, j, k, levels, pixBufStart, pix0, pixBufEnd),
                                                MAXPIXIJK(w, h, x + 1, y, i, j, k, levels, pixBufStart, pix1, pixBufEnd),
                                                MAXPIXIJK(w, h, x + 2, y, i, j, k, levels, pixBufStart, pix2, pixBufEnd),
                                                MAXPIXIJK(w, h, x + 3, y, i, j, k, levels, pixBufStart, pix3, pixBufEnd));

                            originalPix = _mm_min_ps(originalPix, comparisonPix);
                        }
                    }
                }
            }

            _mm_storer_ps(processedResultsBuffer, originalPix);

            x += 4;
            pixCalculate += 4;
            processedResultsBuffer += 4;
        }

        (*chunksProcessed)++;
        return;

    }

}

//Same as above but max of neighbouring pixels rather than min
void dilateChunk(unsigned w, unsigned h, int levels, uint32_t pixPerChunk, uint32_t chunksPerLine, uint64_t *chunksProcessed, float *pixBufStart, float *pixCalculate, float *pixBufEnd, float *processedResultsBuffer)
{
    unsigned x;
    unsigned y;
    calculateChunkXY(w, h, &x, &y, *chunksProcessed, chunksPerLine, pixPerChunk);

    if (levels == 1)
    {
        for (int i = 0; i < pixPerChunk / 4; i++)
        {
            float *pix0 = &pixCalculate[0];
            float *pix1 = &pixCalculate[1];
            float *pix2 = &pixCalculate[2];
            float *pix3 = &pixCalculate[3];

            __m128 originalPix = _mm_set_ps(*pix0, *pix1, *pix2, *pix3);

            __m128 comparisonPix;

            //Above
            comparisonPix = _mm_set_ps(
                                MINPIX(w, h, x, y, 0, -1, pixBufStart, pix0, pixBufEnd),
                                MINPIX(w, h, x + 1, y, 0, -1, pixBufStart, pix1, pixBufEnd),
                                MINPIX(w, h, x + 2, y, 0, -1, pixBufStart, pix2, pixBufEnd),
                                MINPIX(w, h, x + 3, y, 0, -1, pixBufStart, pix3, pixBufEnd));

            originalPix = _mm_max_ps(originalPix, comparisonPix);

            //Below
            comparisonPix = _mm_set_ps(
                                MINPIX(w, h, x, y, 0, 1, pixBufStart, pix0, pixBufEnd),
                                MINPIX(w, h, x + 1, y, 0, 1, pixBufStart, pix1, pixBufEnd),
                                MINPIX(w, h, x + 2, y, 0, 1, pixBufStart, pix2, pixBufEnd),
                                MINPIX(w, h, x + 3, y, 0, 1, pixBufStart, pix3, pixBufEnd));

            originalPix = _mm_max_ps(originalPix, comparisonPix);

            //Left
            comparisonPix = _mm_set_ps(
                                MINPIX(w, h, x, y, -1, 0, pixBufStart, pix0, pixBufEnd),
                                MINPIX(w, h, x + 1, y, -1, 0, pixBufStart, pix1, pixBufEnd),
                                MINPIX(w, h, x + 2, y, -1, 0, pixBufStart, pix2, pixBufEnd),
                                MINPIX(w, h, x + 3, y, -1, 0, pixBufStart, pix3, pixBufEnd));

            originalPix = _mm_max_ps(originalPix, comparisonPix);

            //Right
            comparisonPix = _mm_set_ps(
                                MINPIX(w, h, x, y, 1, 0, pixBufStart, pix0, pixBufEnd),
                                MINPIX(w, h, x + 1, y, 1, 0, pixBufStart, pix1, pixBufEnd),
                                MINPIX(w, h, x + 2, y, 1, 0, pixBufStart, pix2, pixBufEnd),
                                MINPIX(w, h, x + 3, y, 1, 0, pixBufStart, pix3, pixBufEnd));

            originalPix = _mm_max_ps(originalPix, comparisonPix);

            //Store the result
            _mm_storer_ps(processedResultsBuffer, originalPix);

            x += 4;
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
            float *pix0 = &pixCalculate[0];
            float *pix1 = &pixCalculate[1];
            float *pix2 = &pixCalculate[2];
            float *pix3 = &pixCalculate[3];

            __m128 originalPix = _mm_set_ps(*pix0, *pix1, *pix2, *pix3);

            __m128 comparisonPix;

            for (int i = 0; i < levels + 1; i++)
            {
                for (int j = 0; j < levels + 1; j++)
                {
                    comparisonPix = _mm_set_ps(
                                        MINPIXIJK(w, h, x, y, i, j, 0, levels, pixBufStart, pix0, pixBufEnd),
                                        MINPIXIJK(w, h, x + 1, y, i, j, 0, levels, pixBufStart, pix1, pixBufEnd),
                                        MINPIXIJK(w, h, x + 2, y, i, j, 0, levels, pixBufStart, pix2, pixBufEnd),
                                        MINPIXIJK(w, h, x + 3, y, i, j, 0, levels, pixBufStart, pix3, pixBufEnd));

                    originalPix = _mm_max_ps(originalPix, comparisonPix);

                    if (i == 0)
                    {
                        for (int k = 1; k < 2 * j; k += 2)
                        {
                            comparisonPix = _mm_set_ps(
                                                MINPIXIJK(w, h, x, y, i, j, k, levels, pixBufStart, pix0, pixBufEnd),
                                                MINPIXIJK(w, h, x + 1, y, i, j, k, levels, pixBufStart, pix1, pixBufEnd),
                                                MINPIXIJK(w, h, x + 2, y, i, j, k, levels, pixBufStart, pix2, pixBufEnd),
                                                MINPIXIJK(w, h, x + 3, y, i, j, k, levels, pixBufStart, pix3, pixBufEnd));

                            originalPix = _mm_max_ps(originalPix, comparisonPix);

                        }
                    }
                    else if (i < j && i != 0 && j == levels)
                    {
                        for (int k = 1; k < 2 * (levels - i); k += 2)
                        {
                            comparisonPix = _mm_set_ps(
                                                MINPIXIJK(w, h, x, y, i, j, k, levels, pixBufStart, pix0, pixBufEnd),
                                                MINPIXIJK(w, h, x + 1, y, i, j, k, levels, pixBufStart, pix1, pixBufEnd),
                                                MINPIXIJK(w, h, x + 2, y, i, j, k, levels, pixBufStart, pix2, pixBufEnd),
                                                MINPIXIJK(w, h, x + 3, y, i, j, k, levels, pixBufStart, pix3, pixBufEnd));

                            originalPix = _mm_max_ps(originalPix, comparisonPix);
                        }
                    }
                }
            }

            _mm_storer_ps(processedResultsBuffer, originalPix);

            x += 4;
            pixCalculate += 4;
            processedResultsBuffer += 4;
        }

        (*chunksProcessed)++;
        return;

    }

}

//Same as above but doubles instead of floats, gets min of neighbouring pixels
void erodeChunk32(unsigned w, unsigned h, int levels, uint32_t pixPerChunk, uint32_t chunksPerLine, uint64_t *chunksProcessed, double *pixBufStart, double *pixCalculate, double *pixBufEnd, double *processedResultsBuffer)
{
    unsigned x;
    unsigned y;
    calculateChunkXY(w, h, &x, &y, *chunksProcessed, chunksPerLine, pixPerChunk);

    //Cross Shape
    if (levels == 1)
    {
        //This time we can only deal with 2 pixels at a time as they're doubles... If only we had AVX...
        for (int i = 0; i < pixPerChunk / 2; i++)
        {
            double *pix0 = &pixCalculate[0];
            double *pix1 = &pixCalculate[1];

            __m128d originalPix = _mm_set_pd(*pix0, *pix1);

            __m128d comparisonPix;

            //Above
            comparisonPix = _mm_set_pd(
                                MAXPIX32(w, h, x, y, 0, -1, pixBufStart, pix0, pixBufEnd),
                                MAXPIX32(w, h, x + 1, y, 0, -1, pixBufStart, pix1, pixBufEnd));

            originalPix = _mm_min_pd(originalPix, comparisonPix);

            //Below
            comparisonPix = _mm_set_pd(
                                MAXPIX32(w, h, x, y, 0, 1, pixBufStart, pix0, pixBufEnd),
                                MAXPIX32(w, h, x + 1, y, 0, 1, pixBufStart, pix1, pixBufEnd));

            originalPix = _mm_min_pd(originalPix, comparisonPix);

            //Left
            comparisonPix = _mm_set_pd(
                                MAXPIX32(w, h, x, y, -1, 0, pixBufStart, pix0, pixBufEnd),
                                MAXPIX32(w, h, x + 1, y, -1, 0, pixBufStart, pix1, pixBufEnd));

            originalPix = _mm_min_pd(originalPix, comparisonPix);

            //Right
            comparisonPix = _mm_set_pd(
                                MAXPIX32(w, h, x, y, 1, 0, pixBufStart, pix0, pixBufEnd),
                                MAXPIX32(w, h, x + 1, y, 1, 0, pixBufStart, pix1, pixBufEnd));

            originalPix = _mm_min_pd(originalPix, comparisonPix);

            //Store the result
            _mm_storer_pd(processedResultsBuffer, originalPix);

            x += 2;
            pixCalculate += 2;
            processedResultsBuffer += 2;

        }

        (*chunksProcessed)++;
        return;

    }
    //Diamond
    else
    {
        for (int i = 0; i < pixPerChunk / 2; i++)
        {
            double *pix0 = &pixCalculate[0];
            double *pix1 = &pixCalculate[1];

            __m128d originalPix = _mm_set_pd(*pix0, *pix1);

            __m128d comparisonPix;

            for (int i = 0; i < levels + 1; i++)
            {
                for (int j = 0; j < levels + 1; j++)
                {
                    comparisonPix = _mm_set_pd(
                                        MAXPIXIJK32(w, h, x, y, i, j, 0, levels, pixBufStart, pix0, pixBufEnd),
                                        MAXPIXIJK32(w, h, x + 1, y, i, j, 0, levels, pixBufStart, pix1, pixBufEnd));

                    originalPix = _mm_min_pd(originalPix, comparisonPix);

                    if (i == 0)
                    {
                        for (int k = 1; k < 2 * j; k += 2)
                        {
                            comparisonPix = _mm_set_pd(
                                                MAXPIXIJK32(w, h, x, y, i, j, k, levels, pixBufStart, pix0, pixBufEnd),
                                                MAXPIXIJK32(w, h, x + 1, y, i, j, k, levels, pixBufStart, pix1, pixBufEnd));

                            originalPix = _mm_min_pd(originalPix, comparisonPix);

                        }
                    }
                    else if (i < j && i != 0 && j == levels)
                    {
                        for (int k = 1; k < 2 * (levels - i); k += 2)
                        {
                            comparisonPix = _mm_set_pd(
                                                MAXPIXIJK32(w, h, x, y, i, j, k, levels, pixBufStart, pix0, pixBufEnd),
                                                MAXPIXIJK32(w, h, x + 1, y, i, j, k, levels, pixBufStart, pix1, pixBufEnd));

                            originalPix = _mm_min_pd(originalPix, comparisonPix);
                        }
                    }
                }
            }

            _mm_storer_pd(processedResultsBuffer, originalPix);

            x += 2;
            pixCalculate += 2;
            processedResultsBuffer += 2;
        }

        (*chunksProcessed)++;
        return;

    }

}

//Same as above but doubles instead of floats, gets max of neighbouring pixels
void dilateChunk32(unsigned w, unsigned h, int levels, uint32_t pixPerChunk, uint32_t chunksPerLine, uint64_t *chunksProcessed, double *pixBufStart, double *pixCalculate, double *pixBufEnd, double *processedResultsBuffer)
{
    unsigned x;
    unsigned y;
    calculateChunkXY(w, h, &x, &y, *chunksProcessed, chunksPerLine, pixPerChunk);

    if (levels == 1)
    {
        for (int i = 0; i < pixPerChunk / 2; i++)
        {
            double *pix0 = &pixCalculate[0];
            double *pix1 = &pixCalculate[1];

            __m128d originalPix = _mm_set_pd(*pix0, *pix1);

            __m128d comparisonPix;

            //Above
            comparisonPix = _mm_set_pd(
                                MINPIX32(w, h, x, y, 0, -1, pixBufStart, pix0, pixBufEnd),
                                MINPIX32(w, h, x + 1, y, 0, -1, pixBufStart, pix1, pixBufEnd));

            originalPix = _mm_max_pd(originalPix, comparisonPix);

            //Below
            comparisonPix = _mm_set_pd(
                                MINPIX32(w, h, x, y, 0, 1, pixBufStart, pix0, pixBufEnd),
                                MINPIX32(w, h, x + 1, y, 0, 1, pixBufStart, pix1, pixBufEnd));

            originalPix = _mm_max_pd(originalPix, comparisonPix);

            //Left
            comparisonPix = _mm_set_pd(
                                MINPIX32(w, h, x, y, -1, 0, pixBufStart, pix0, pixBufEnd),
                                MINPIX32(w, h, x + 1, y, -1, 0, pixBufStart, pix1, pixBufEnd));

            originalPix = _mm_max_pd(originalPix, comparisonPix);

            //Right
            comparisonPix = _mm_set_pd(
                                MINPIX32(w, h, x, y, 1, 0, pixBufStart, pix0, pixBufEnd),
                                MINPIX32(w, h, x + 1, y, 1, 0, pixBufStart, pix1, pixBufEnd));

            originalPix = _mm_max_pd(originalPix, comparisonPix);

            //Store the result
            _mm_storer_pd(processedResultsBuffer, originalPix);

            x += 2;
            pixCalculate += 2;
            processedResultsBuffer += 2;

        }

        (*chunksProcessed)++;
        return;

    }
    //Diamond
    else
    {
        for (int i = 0; i < pixPerChunk / 2; i++)
        {
            double *pix0 = &pixCalculate[0];
            double *pix1 = &pixCalculate[1];

            __m128d originalPix = _mm_set_pd(*pix0, *pix1);

            __m128d comparisonPix;

            for (int i = 0; i < levels + 1; i++)
            {
                for (int j = 0; j < levels + 1; j++)
                {
                    comparisonPix = _mm_set_pd(
                                        MINPIXIJK32(w, h, x, y, i, j, 0, levels, pixBufStart, pix0, pixBufEnd),
                                        MINPIXIJK32(w, h, x + 1, y, i, j, 0, levels, pixBufStart, pix1, pixBufEnd));

                    originalPix = _mm_max_pd(originalPix, comparisonPix);

                    if (i == 0)
                    {
                        for (int k = 1; k < 2 * j; k += 2)
                        {
                            comparisonPix = _mm_set_pd(
                                                MINPIXIJK32(w, h, x, y, i, j, k, levels, pixBufStart, pix0, pixBufEnd),
                                                MINPIXIJK32(w, h, x + 1, y, i, j, k, levels, pixBufStart, pix1, pixBufEnd));

                            originalPix = _mm_max_pd(originalPix, comparisonPix);

                        }
                    }
                    else if (i < j && i != 0 && j == levels)
                    {
                        for (int k = 1; k < 2 * (levels - i); k += 2)
                        {
                            comparisonPix = _mm_set_pd(
                                                MINPIXIJK32(w, h, x, y, i, j, k, levels, pixBufStart, pix0, pixBufEnd),
                                                MINPIXIJK32(w, h, x + 1, y, i, j, k, levels, pixBufStart, pix1, pixBufEnd));

                            originalPix = _mm_max_pd(originalPix, comparisonPix);
                        }
                    }
                }
            }

            _mm_storer_pd(processedResultsBuffer, originalPix);

            x += 2;
            pixCalculate += 2;
            processedResultsBuffer += 2;
        }

        (*chunksProcessed)++;
        return;

    }

}

//Check if pixel to compare is in image
bool validPixel(unsigned w, unsigned h, unsigned x, unsigned y, int dx, int dy)
{
    //If we're off the left or right edge
    if (x + dx < 0 || x + dx >= w)
    {
        return false;
    }
    //Else if we're off the top or bottom
    else if (y + dy < 0 || y + dy >= h)
    {
        return false;
    }
    //Else, great success, we're trying to read a valid pixel!
    else
    {
        return true;
    }
}

float getPixel(unsigned w, unsigned h, unsigned x, unsigned y, int dx, int dy, float *memStart, float *pixPtr, float *memEnd)
{
    //Calculate offset to original pixel pointer
    int offset = dx + (w * dy);
    float *result = pixPtr + offset;

    //If it's outside scope of memory, wrap it back around
    if (result < memStart)
    {
        result = result + ((memEnd - memStart));
    }
    else if (result >= memEnd)
    {
        result = memStart + ((result - memEnd));
    }

    //Return the value of the pixel
    return *result;

}

//Same as above but doubles instead of floats
double getPixel32(unsigned w, unsigned h, unsigned x, unsigned y, int dx, int dy, double *memStart, double *pixPtr, double *memEnd)
{
    int offset = dx + (w * dy);
    double *result = pixPtr + offset;

    if (result < memStart)
    {
        result = result + ((memEnd - memStart));
    }
    else if (result >= memEnd)
    {
        result = memStart + ((result - memEnd));
    }

    return *result;

}

//          ___
//     i___|0,0|___ j
//  ___|1,0|___|0,1|___
// |2,0|___|1,1|___|0,2|
//     |2,1|___|1,2|
//         |2,2|
//              ___
//          ___|_1_|___
//      ___|11_|_3_|_2_|___
//  ___|17_|_6_|12_|_5_|_4_|___
// |22_|10_|18_|_9_|13_|_8_|_7_|
//     |23_|16_|19_|15_|14_|
//         |24_|21_|20_|
//             |25_|

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

//Process chunk of the original data and write it to intermediate buffer, then if we have enough data in the intermediate buffer, process a final result chunk
bool processStreamChunk(unsigned w, unsigned h, bool closed, int levels, uint32_t pixPerChunk, uint32_t chunksPerLine, uint32_t totalChunks, uint64_t *originalImgChunksProcessed, float *pixBufStart, float **pixCalculate, float *pixBufEnd, uint64_t *irChunksProcessed, float *irStart, float **irInsert, float **irCalculate, float *irEnd, float *finalResult)
{
    //If we're doing dilate first
    if (!closed)
    {
        //If we've not finished processing the whole input image
        if (*originalImgChunksProcessed < (totalChunks))
        {
            //Do the dilate operation
            dilateChunk(w, h, levels, pixPerChunk, chunksPerLine, originalImgChunksProcessed, pixBufStart, *pixCalculate, pixBufEnd, *irInsert);

            //Increment where we're next processing in the cyclic pixel buffer
            (*pixCalculate) += pixPerChunk;
            //Wrap if we venture off the end
            if (*pixCalculate >= pixBufEnd)
            {
                *pixCalculate = pixBufStart + (((*pixCalculate) - pixBufEnd) / sizeof(float));
            }

            //Do the same for where we're next inserting into the intermediate results buffer
            (*irInsert) += pixPerChunk;
            if (*irInsert >= irEnd)
            {
                (*irInsert) = irStart + (((*irInsert) - irEnd) / sizeof(float));
            }
        }
        //If we've read enough data to begin processing the intermediate buffer
        if (*originalImgChunksProcessed > (levels * chunksPerLine))
        {
            //Do the erosion
            erodeChunk(w, h, levels, pixPerChunk, chunksPerLine, irChunksProcessed, irStart, *irCalculate, irEnd, finalResult);

            //Increment where we're processing in the intermediate buffer next
            (*irCalculate) += pixPerChunk;
            //If we go off the end, wrap back to the beginning
            if (*irCalculate >= irEnd)
            {
                (*irCalculate) = irStart + (((*irCalculate) - irEnd) / sizeof(float));
            }
            //We have something to pack and write out
            return true;
        }
        //We don't have anything to pack and write out
        return false;
    }
    //Same as above but other way around
    else
    {
        if (*originalImgChunksProcessed < (totalChunks))
        {
            erodeChunk(w, h, levels, pixPerChunk, chunksPerLine, originalImgChunksProcessed, pixBufStart, *pixCalculate, pixBufEnd, *irInsert);

            (*pixCalculate) += pixPerChunk;
            if (*pixCalculate >= pixBufEnd)
            {
                (*pixCalculate) = pixBufStart + (((*pixCalculate) - pixBufEnd) / sizeof(float));
            }

            (*irInsert) += pixPerChunk;
            if (*irInsert >= irEnd)
            {
                (*irInsert) = irStart + ((*irInsert - irEnd) / sizeof(float));
            }
        }
        if (*originalImgChunksProcessed > (levels * chunksPerLine))
        {
            dilateChunk(w, h, levels, pixPerChunk, chunksPerLine, irChunksProcessed, irStart, *irCalculate, irEnd, finalResult);

            (*irCalculate) += pixPerChunk;
            if (*irCalculate >= irEnd)
            {
                (*irCalculate) = irStart + (((*irCalculate) - irEnd) / sizeof(float));
            }

            return true;
        }

        return false;
    }
}

//Same as above but uses doubles instead of floats
bool processStreamChunk32(unsigned w, unsigned h, bool closed, int levels, uint32_t pixPerChunk, uint32_t chunksPerLine, uint32_t totalChunks, uint64_t *originalImgChunksProcessed, double *pixBufStart, double **pixCalculate, double *pixBufEnd, uint64_t *irChunksProcessed, double *irStart, double **irInsert, double **irCalculate, double *irEnd, double *finalResult)
{
    if (!closed)
    {
        if (*originalImgChunksProcessed < (totalChunks))
        {
            dilateChunk32(w, h, levels, pixPerChunk, chunksPerLine, originalImgChunksProcessed, pixBufStart, *pixCalculate, pixBufEnd, *irInsert);

            (*pixCalculate) += pixPerChunk;
            if (*pixCalculate >= pixBufEnd)
            {
                *pixCalculate = pixBufStart + (((*pixCalculate) - pixBufEnd) / sizeof(double));
            }

            (*irInsert) += pixPerChunk;
            if (*irInsert >= irEnd)
            {
                (*irInsert) = irStart + (((*irInsert) - irEnd) / sizeof(double));
            }
        }
        if (*originalImgChunksProcessed > (levels * chunksPerLine))
        {
            erodeChunk32(w, h, levels, pixPerChunk, chunksPerLine, irChunksProcessed, irStart, *irCalculate, irEnd, finalResult);

            (*irCalculate) += pixPerChunk;
            if (*irCalculate >= irEnd)
            {
                (*irCalculate) = irStart + (((*irCalculate) - irEnd) / sizeof(double));
            }

            return true;
        }

        return false;
    }
    else
    {
        if (*originalImgChunksProcessed < (totalChunks))
        {
            erodeChunk32(w, h, levels, pixPerChunk, chunksPerLine, originalImgChunksProcessed, pixBufStart, *pixCalculate, pixBufEnd, *irInsert);

            (*pixCalculate) += pixPerChunk;
            if (*pixCalculate >= pixBufEnd)
            {
                (*pixCalculate) = pixBufStart + (((*pixCalculate) - pixBufEnd) / sizeof(double));
            }

            (*irInsert) += pixPerChunk;
            if (*irInsert >= irEnd)
            {
                (*irInsert) = irStart + ((*irInsert - irEnd) / sizeof(double));
            }
        }
        if (*originalImgChunksProcessed > (levels * chunksPerLine))
        {
            dilateChunk32(w, h, levels, pixPerChunk, chunksPerLine, irChunksProcessed, irStart, *irCalculate, irEnd, finalResult);

            (*irCalculate) += pixPerChunk;
            if (*irCalculate >= irEnd)
            {
                (*irCalculate) = irStart + (((*irCalculate) - irEnd) / sizeof(double));
            }

            return true;
        }

        return false;
    }
}
