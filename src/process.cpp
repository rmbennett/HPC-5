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

// You may want to play with this to check you understand what is going on
void invert(int levels, unsigned w, unsigned h, unsigned bits, std::vector<uint32_t> &pixels)
{
    uint32_t mask = 0xFFFFFFFFul >> bits;

    for (unsigned i = 0; i < w * h; i++)
    {
        pixels[i] = mask - pixels[i];
    }
}

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

        float *processedResults = (float *)malloc(sizeof(float) * pixPerChunk);

        uint32_t *readBuffer = (uint32_t *)malloc(sizeof(uint32_t) * bytesToRead);
        uint32_t *resultBuffer = (uint32_t *)malloc(sizeof(uint32_t) * bytesToRead);
        uint64_t chunksRead = 0;
        uint64_t chunksProcessed = 0;

        bool readingInput = true;

        while (1)
        {

            if (readingInput)
            {
                // fprintf(stderr, "Bytes to read %d\n", bytesToRead);
                if (!read_chunk(STDIN_FILENO, bytesToRead, readBuffer))
                    readingInput = false;

                // fprintf(stderr, "Old Pointer: %d %d\n", pixBufInsert, pixBufStart);

                unpack_chunk(bytesToRead, bits, &chunksRead, bitMask, readBuffer, pixBufStart, &pixBufInsert, pixBufEnd);

                // fprintf(stderr, "New Pointer: %d, diff: %d\n", pixBufInsert, pixBufInsert - pixBufStart);

                // fprintf(stderr, "Some stuff: %d, chunksRead %d\n", ((bytesToRead * 8) / bits), chunksRead);
            }

            if (chunksRead > chunksPerLine * levels)
            {
                // float *resPtr = pixBufInsert - ((bytesToRead * 8) / bits);
                // if (resPtr < pixBufStart)
                // {
                //     resPtr = pixBufEnd - (pixBufStart - resPtr);
                // }
                // fprintf(stderr, "resPtr %d, start %d\n", resPtr, pixBufStart);

                // fprintf(stderr, "%lf %lf %lf %lf\n", resPtr[0], resPtr[1], resPtr[2], resPtr[3]);
                // // fprintf(stderr, "%d %d %d %d\n", pixBufInsert, resPtr, pixBufInsert - resPtr, (bytesToRead * 8) / bits);

                // processStream(w, h, levels, pixPerChunk, chunksPerLine, &chunksProcessed, chunksRead, pixBufStart, &pixBufCalculate, pixBufEnd, processedResults);

                pack_chunk(bytesToRead, bits, bitMask, resultBuffer, processedResults);

                // fprintf(stderr, "%x %x\n", readBuffer[0], resultBuffer[0]);

                // sleep(5);

                // fprintf(stderr, "%lf\n", pixBufInsert[-1]);

                if (!write_chunk(STDOUT_FILENO, bytesToRead, resultBuffer))
                    break;
            }
        }

        // fprintf(stderr, "%ld %ld\n", pixBufStart, pixBufEnd);

        //  uint64_t cbRaw=uint64_t(w)*h*bits/8;
        //  std::vector<uint64_t> raw(cbRaw/8);

        //  std::vector<uint32_t> pixels(w*h);
        //  s1 = stamp();
        //  while(1)
        //  {
        //      //s3 = stamp();
        //      if(!read_blob(STDIN_FILENO, cbRaw, &raw[0]))
        //          break;  // No more images
        //      //s4 = stamp();
        //      unpack_blob(w, h, bits, &raw[0], &pixels[0]);
        //      //s5 = stamp();
        //      process(levels, w, h, bits, pixels);
        //      //invert(levels, w, h, bits, pixels);
        //      //s6 = stamp();
        //      pack_blob(w, h, bits, &pixels[0], &raw[0]);
        //      //s7 = stamp();
        //      write_blob(STDOUT_FILENO, cbRaw, &raw[0]);
        //  }
        //  s2 = stamp();
        //  // fprintf(stderr,"Overall time %g s\n", (s2 - s1) / 1e9);
        //  // fprintf(stderr,"Read Blob %g s\n", (s4 - s3) / 1e9);
        //  // fprintf(stderr,"unpack_blob %g s\n", (s5 - s4) / 1e9);
        //  // fprintf(stderr, "process %g s\n", (s6 - s5) / 1e9);
        //  // fprintf(stderr,"pack_blob %g s\n", (s7 - s6) / 1e9);
        //  // fprintf(stderr,"write_blob %g s\n", (s2 - s7) / 1e9);
        //  return 0;
    }
    catch (std::exception &e)
    {
        std::cerr << "Caught exception : " << e.what() << "\n";
        return 1;
    }
}

