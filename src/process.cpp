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

#include "file_utils.hpp"
#include "image_process.hpp"

#define CHUNKSIZE 64

//Timing function - from Advanced Computer Architecture - Coursework 2 - Prof. Kelly
long stamp()
{
    struct timespec tv;
    long _stamp;
    clock_gettime(CLOCK_MONOTONIC, &tv);
    _stamp = tv.tv_sec * 1000 * 1000 * 1000 + tv.tv_nsec;
    return _stamp;
}

int main(int argc, char *argv[])
{
    long s1, s2, s3, s4, s5, s6, s7;
    try
    {
        if (argc < 3)
        {
            fprintf(stderr, "Usage: process width height [bits] [levels]\n");
            fprintf(stderr, "   bits=8 by default\n");
            fprintf(stderr, "   levels=1 by default\n");
            exit(1);
        }

        unsigned w = atoi(argv[1]);
        unsigned h = atoi(argv[2]);

        unsigned bits = 8;
        if (argc > 3)
        {
            bits = atoi(argv[3]);
        }

        if (bits > 32)
            throw std::invalid_argument("Bits must be <= 32.");

        unsigned tmp = bits;
        while (tmp != 1)
        {
            tmp >>= 1;
            if (tmp == 0)
                throw std::invalid_argument("Bits must be a binary power.");
        }

        if ( ((w * bits) % 64) != 0)
        {
            throw std::invalid_argument(" width*bits must be divisible by 64.");
        }

        int levels = 1;
        if (argc > 4)
        {
            levels = atoi(argv[4]);
        }

        fprintf(stderr, "Processing %d x %d image with %d bits per pixel.\n", w, h, bits);

        uint64_t bytesToRead = sizeof(uint64_t);
        uint32_t pixPerChunk = CHUNKSIZE / bits;

        uint32_t chunksPerLine = w / pixPerChunk;
        uint32_t totalChunks = (w * h) / pixPerChunk;

        uint64_t readChunk;
        uint64_t resultChunk;

        uint64_t chunksRead = 0;
        uint64_t originalChunksProcessed = 0;
        uint64_t irChunksProcessed = 0;

        if (bits < 32)
        {
            uint32_t pixBufSize = sizeof(float) * (w * ((2 * levels) + 1));

            float *pixBufStart = (float *)malloc(pixBufSize);
            float *pixBufEnd = pixBufStart + (pixBufSize / sizeof(float));
            float *pixBufInsert = pixBufStart;
            float *pixBufCalculate = pixBufStart;

            float *irStart = (float *)malloc(pixBufSize);
            float *irEnd = irStart + (pixBufSize / sizeof(float));
            float *irInsert = irStart;
            float *irCalculate = irStart;


            float *finalResult = (float *)malloc(sizeof(float) * pixPerChunk);

            while (1)
            {

                if (chunksRead < (totalChunks))
                {
                    if (!read_blob(STDIN_FILENO, bytesToRead, &readChunk))
                    {
                        break;
                    }
                    unpack_blob(bits, pixPerChunk, &chunksRead, readChunk, pixBufStart, &pixBufInsert, pixBufEnd);
                    // if (!(chunksRead %3*chunksPerLine))
                    //     fprintf(stderr, "Addresses Equal %d\n", pixBufStart == pixBufInsert);
                }

                if (chunksRead > (levels * chunksPerLine))
                {
                    if (processStreamChunk(w, h, levels, pixPerChunk, chunksPerLine, totalChunks, &originalChunksProcessed, pixBufStart, &pixBufCalculate, pixBufEnd, &irChunksProcessed, irStart, &irInsert, &irCalculate, irEnd, finalResult))
                    {
                        pack_blob(bits, pixPerChunk, finalResult, &resultChunk);
                        write_blob(STDOUT_FILENO, bytesToRead, &resultChunk);
                    }
                }

                // fprintf(stderr, "chunksRead %d originalChunksProcessed %d irChunksProcessed %d\n", chunksRead, originalChunksProcessed, irChunksProcessed);
                // fprintf(stderr, "pixBufInsert %d pixBufCalculate %d\n", pixBufInsert, pixBufCalculate);
                // fprintf(stderr, "irInsert %d irCalculate %d\n", irInsert, irCalculate);


                if (chunksRead == (totalChunks) && originalChunksProcessed == (totalChunks) && irChunksProcessed == (totalChunks))
                {
                    chunksRead = 0;
                    pixBufInsert = pixBufStart;
                    pixBufCalculate = pixBufStart;
                    originalChunksProcessed = 0;
                    irInsert = irStart;
                    irCalculate = irStart;
                }
            }
            free (pixBufStart);
            free (irStart);
            free (finalResult);
        }
        else
        {

            uint32_t pixBufSize = sizeof(double) * (w * ((2 * levels) + 1));

            double *pixBufStart = (double *)malloc(pixBufSize);
            double *pixBufEnd = pixBufStart + (pixBufSize / sizeof(double));
            double *pixBufInsert = pixBufStart;
            double *pixBufCalculate = pixBufStart;

            double *irStart = (double *)malloc(pixBufSize);
            double *irEnd = irStart + (pixBufSize / sizeof(double));
            double *irInsert = irStart;
            double *irCalculate = irStart;


            double *finalResult = (double *)malloc(sizeof(double) * pixPerChunk);

            while (1)
            {

                if (chunksRead < (totalChunks))
                {
                    if (!read_blob(STDIN_FILENO, bytesToRead, &readChunk))
                    {
                        break;
                    }
                    unpack_blob32(bits, pixPerChunk, &chunksRead, readChunk, pixBufStart, &pixBufInsert, pixBufEnd);
                    // if (!(chunksRead %3*chunksPerLine))
                    //     fprintf(stderr, "Addresses Equal %d\n", pixBufStart == pixBufInsert);
                }

                if (chunksRead > (levels * chunksPerLine))
                {
                    if (processStreamChunk32(w, h, levels, pixPerChunk, chunksPerLine, totalChunks, &originalChunksProcessed, pixBufStart, &pixBufCalculate, pixBufEnd, &irChunksProcessed, irStart, &irInsert, &irCalculate, irEnd, finalResult))
                    {
                        pack_blob32(bits, pixPerChunk, finalResult, &resultChunk);
                        write_blob(STDOUT_FILENO, bytesToRead, &resultChunk);
                    }
                }

                // fprintf(stderr, "chunksRead %d originalChunksProcessed %d irChunksProcessed %d\n", chunksRead, originalChunksProcessed, irChunksProcessed);
                // fprintf(stderr, "pixBufInsert %d pixBufCalculate %d\n", pixBufInsert, pixBufCalculate);
                // fprintf(stderr, "irInsert %d irCalculate %d\n", irInsert, irCalculate);


                if (chunksRead == (totalChunks) && originalChunksProcessed == (totalChunks) && irChunksProcessed == (totalChunks))
                {
                    chunksRead = 0;
                    pixBufInsert = pixBufStart;
                    pixBufCalculate = pixBufStart;
                    originalChunksProcessed = 0;
                    irInsert = irStart;
                    irCalculate = irStart;
                }
            }
            free (pixBufStart);
            free (irStart);
            free (finalResult);
        }
    }

    catch (std::exception &e)
    {
        std::cerr << "Caught exception : " << e.what() << "\n";
        return 1;
    }
}

