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

        fprintf(stderr, "%d\n", bytesToRead);

        uint32_t chunksPerLine = w / pixPerChunk;
        uint32_t totalChunks = w * h / pixPerChunk;

        uint32_t pixBufSize = sizeof(float) * ((w * (2 * levels)) + pixPerChunk);

        fprintf(stderr, "%d\n", pixBufSize);

        float *pixBufStart = (float *)malloc(pixBufSize);
        float *pixBufEnd = pixBufStart + pixBufSize / sizeof(float);
        float *pixBufInsert = pixBufStart;
        float *pixBufCalculate = pixBufStart;

        float *irStart = (float *)malloc(pixBufSize);
        float *irEnd = irStart + pixBufSize / sizeof(float);
        float *irInsert = irStart;
        float *irCalculate = irStart;


        float *finalResult = (float *)malloc(sizeof(float) * pixPerChunk);

        uint32_t *readBuffer = (uint32_t *)malloc(sizeof(uint32_t) * bytesToRead);
        uint32_t *resultBuffer = (uint32_t *)malloc(sizeof(uint32_t) * bytesToRead);

        uint64_t chunksRead = 0;
        uint64_t originalChunksProcessed = 0;
        uint64_t irChunksProcessed = 0;

        while (1)
        {

            if (chunksRead < (totalChunks))
            {
                if (!read_chunk(STDIN_FILENO, bytesToRead, readBuffer))
                {
                    break;
                }
                unpack_blob(w, h, bytesToRead, bits, &chunksRead, readBuffer, pixBufStart, &pixBufInsert, pixBufEnd);
            }

            if (chunksRead > (levels * chunksPerLine))
            {
                if (processStreamChunk(w, h, levels, pixPerChunk, chunksPerLine, totalChunks, &originalChunksProcessed, pixBufStart, &pixBufCalculate, pixBufEnd, &irChunksProcessed, irStart, &irInsert, &irCalculate, irEnd, finalResult))
                {
                    pack_chunk(bytesToRead, bits, bitMask, resultBuffer, finalResult);
                    if (!write_chunk(STDOUT_FILENO, bytesToRead, resultBuffer))
                        break;
                }
            }

            if (chunksRead == (totalChunks) && originalChunksProcessed == (totalChunks) && irChunksProcessed == (totalChunks))
            {
                fprintf(stderr, "Here\n");
                chunksRead = 0;
                pixBufInsert = pixBufStart;
                pixBufCalculate = pixBufStart;
                originalChunksProcessed = 0;
                irInsert = irStart;
                irCalculate = irStart;
            }
        }
        // fprintf(stderr, "Here to free\n");
        // free (pixBufStart);
        // fprintf(stderr, "Here to free 1\n");
        // free (irStart);
        // fprintf(stderr, "Here to free 2\n");
        // free (finalResult);
        fprintf(stderr, "Here to free 3\n");
        free (readBuffer);
        fprintf(stderr, "Here to free 4\n");
        free(resultBuffer);
    }

    catch (std::exception &e)
    {
        std::cerr << "Caught exception : " << e.what() << "\n";
        return 1;
    }
}

