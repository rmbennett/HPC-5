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

uint64_t shuffle64(unsigned bits, uint64_t x)
{
    if (bits == 1)
    {
        x = ((x & 0x0101010101010101ull) << 7)
            | ((x & 0x0202020202020202ull) << 5)
            | ((x & 0x0404040404040404ull) << 3)
            | ((x & 0x0808080808080808ull) << 1)
            | ((x & 0x1010101010101010ull) >> 1)
            | ((x & 0x2020202020202020ull) >> 3)
            | ((x & 0x4040404040404040ull) >> 5)
            | ((x & 0x8080808080808080ull) >> 7);
    }
    else if (bits == 2)
    {
        x = ((x & 0x0303030303030303ull) << 6)
            | ((x & 0x0c0c0c0c0c0c0c0cull) << 2)
            | ((x & 0x3030303030303030ull) >> 2)
            | ((x & 0xc0c0c0c0c0c0c0c0ull) >> 6);
    }
    else if (bits == 4)
    {
        x = ((x & 0x0f0f0f0f0f0f0f0full) << 4)
            | ((x & 0xf0f0f0f0f0f0f0f0ull) >> 4);
    }
    return x;
}

/*! Take data packed into incoming format, and expand to one float per pixel */
void unpack_blob(unsigned bits, uint32_t pixPerChunk, uint64_t *chunksRead, const uint64_t pRaw, float *pixBufStart, float **pixBufInsertPtr, float *pixBufEnd)
{
    uint64_t buffer = 0;
    unsigned bufferedBits = 0;

    const uint64_t MASK = 0xFFFFFFFFFFFFFFFFULL >> (64 - bits);

    buffer = shuffle64(bits, pRaw);
    bufferedBits = 64;

	// fprintf(stderr, "Before: ");
    for (unsigned i = 0; i < pixPerChunk; i++)
    {
        **pixBufInsertPtr = (float)(uint64_t(buffer & MASK));
        // fprintf(stderr, "%d ", uint32_t(**pixBufInsertPtr));
        // if(i != 0 && !((i+1)%8) && i != pixPerChunk - 1)
        // 	fprintf(stderr, "\nBefore: ");
        (*pixBufInsertPtr)++;
        if ((*pixBufInsertPtr) >= pixBufEnd)
        {
            *pixBufInsertPtr = pixBufStart + ((*pixBufInsertPtr - pixBufEnd) / sizeof(float));
        }
        buffer = buffer >> bits;
        bufferedBits -= bits;
    }
    // fprintf(stderr, "\n");

    (*chunksRead)++;
    assert(bufferedBits == 0);
}
/*! Take data packed into incoming format, and expand to one double per pixel - for 32bit images. */
void unpack_blob32(unsigned bits, uint32_t pixPerChunk, uint64_t *chunksRead, const uint64_t pRaw, double *pixBufStart, double **pixBufInsertPtr, double *pixBufEnd)
{
    uint64_t buffer = 0;
    unsigned bufferedBits = 0;

    const uint64_t MASK = 0xFFFFFFFFFFFFFFFFULL >> (64 - bits);

    buffer = shuffle64(bits, pRaw);
    bufferedBits = 64;

	// fprintf(stderr, "Before: ");

    for (unsigned i = 0; i < pixPerChunk; i++)
    {
        **pixBufInsertPtr = (double)(uint64_t(buffer & MASK));
        // fprintf(stderr, "%d ", uint32_t(**pixBufInsertPtr));
        // if(i != 0 && !((i+1)%8) && i != pixPerChunk - 1)
        // 	fprintf(stderr, "\nBefore: ");
        (*pixBufInsertPtr)++;
        if ((*pixBufInsertPtr) >= pixBufEnd)
        {
            *pixBufInsertPtr = pixBufStart + ((*pixBufInsertPtr - pixBufEnd) / sizeof(double));
        }
        buffer = buffer >> bits;
        bufferedBits -= bits;
    }
    // fprintf(stderr, "\n");

    (*chunksRead)++;
    assert(bufferedBits == 0);
}

/*! Go back from one float per pixel to packed format for output. */
void pack_blob(unsigned bits, uint32_t pixPerChunk, const float *pUnpacked, uint64_t *pRaw)
{
    uint64_t buffer = 0;
    unsigned bufferedBits = 0;

    const uint64_t MASK = 0xFFFFFFFFFFFFFFFFULL >> (64 - bits);

    for (unsigned i = 0; i < pixPerChunk; i++)
    {

        buffer = buffer | ((uint64_t(pUnpacked[i]) & MASK) << bufferedBits);
        bufferedBits += bits;

        if (bufferedBits == 64)
        {
            *pRaw = shuffle64(bits, buffer);
            buffer = 0;
            bufferedBits = 0;
        }
    }

    assert(bufferedBits == 0);
}
/*! Go back from one double per pixel to packed format for output. */
void pack_blob32(unsigned bits, uint32_t pixPerChunk, const double *pUnpacked, uint64_t *pRaw)
{
    uint64_t buffer = 0;
    unsigned bufferedBits = 0;

    const uint64_t MASK = 0xFFFFFFFFFFFFFFFFULL >> (64 - bits);

    for (unsigned i = 0; i < pixPerChunk; i++)
    {

        buffer = buffer | ((uint64_t(pUnpacked[i]) & MASK) << bufferedBits);
        bufferedBits += bits;

        if (bufferedBits == 64)
        {
            *pRaw = shuffle64(bits, buffer);
            buffer = 0;
            bufferedBits = 0;
        }
    }

    assert(bufferedBits == 0);
}


bool read_blob(int fd, uint64_t cbBlob, void *pBlob)
{
    uint8_t *pBytes = (uint8_t *)pBlob;

    uint64_t done = 0;
    while (done < cbBlob)
    {
        int todo = (int)std::min(uint64_t(1) << 30, cbBlob - done);

        int got = read(fd, pBytes + done, todo);
        if (got == 0 && done == 0)
            return false;   // end of file
        if (got <= 0)
            throw std::invalid_argument("Read failure.");
        done += got;
    }

    return true;
}

void write_blob(int fd, uint64_t cbBlob, const void *pBlob)
{
    const uint8_t *pBytes = (const uint8_t *)pBlob;

    uint64_t done = 0;
    while (done < cbBlob)
    {
        int todo = (int)std::min(uint64_t(1) << 30, cbBlob - done);

        int got = write(fd, pBytes + done, todo);
        if (got <= 0)
            throw std::invalid_argument("Write failure.");
        done += got;
    }
}