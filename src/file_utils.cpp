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

/*! Take data packed into incoming format, and exand to one integer per pixel */
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

/*! Go back from one integer per pixel to packed format for output. */
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



// bool read_chunk(int fd, uint32_t bytesToRead, uint32_t *readBuffer)
// {

//     int got = read(fd, readBuffer, bytesToRead);
//     if (got != bytesToRead)
//         return false;   // end of file
//     if (got <= 0)
//         throw std::invalid_argument("Read failure.");

//     return true;

// }

// uint32_t shuffle32(unsigned bits, uint32_t x)
// {
//  if(bits==1){
//      x=((x&0x01010101ul)<<7)
//          | ((x&0x02020202ul)<<5)
//          | ((x&0x04040404ul)<<3)
//          | ((x&0x08080808ul)<<1)
//          | ((x&0x10101010ul)>>1)
//          | ((x&0x20202020ul)>>3)
//          | ((x&0x40404040ul)>>5)
//          | ((x&0x80808080ul)>>7);
//  }else if(bits==2){
//      x=((x&0x03030303ull)<<6)
//          | ((x&0x0c0c0c0cul)<<2)
//          | ((x&0x30303030ul)>>2)
//          | ((x&0xc0c0c0c0ul)>>6);
//  }else if(bits==4){
//      x=((x&0x0f0f0f0ful)<<4)
//          | ((x&0xf0f0f0f0ul)>>4);
//  }
//  return x;
// }

/*! Take data packed into incoming format, and exand to one integer per pixel */
// void unpack_blob(unsigned w, unsigned h, uint32_t bytesToRead, unsigned bits, uint64_t *chunksRead, uint32_t *readBuffer, float *pixBufStart, float **pixBufInsertPtr, float *pixBufEnd)
// {
//  uint32_t buffer=0;
//  unsigned bufferedBits=0;

//  const uint32_t MASK=0xFFFFFFFFUL>>(32-bits);

//  for(unsigned i=0;i<(bytesToRead * 8) / bits;i++){
//      if(bufferedBits==0){
//          buffer=shuffle32(bits, *readBuffer++);
//          bufferedBits=32;
//      }

//      **pixBufInsertPtr=(float)(buffer&MASK);
//      // fprintf(stderr, "unpacked %f\n", **pixBufInsertPtr);
//      // sleep(1);
//      (*pixBufInsertPtr)++;
//      if ((*pixBufInsertPtr) >= pixBufEnd)
//      {
//          // fprintf(stderr, "Wrapping from %d\n", *pixBufInsertPtr);
//          *pixBufInsertPtr = pixBufStart + ((*pixBufInsertPtr - pixBufEnd)/sizeof(float));
//          // fprintf(stderr, "To %d, chunksRead %d, memstart %d memend %d\n", *pixBufInsertPtr, *chunksRead, pixBufStart, pixBufEnd);
//      }
//      buffer=buffer>>bits;
//      bufferedBits-=bits;
//  }

//  (*chunksRead)++;

// }


// void pack_chunk(uint32_t bytesRead, unsigned bits, unsigned mask, uint32_t *resultBuffer, float *resultsToPack)
// {
//     uint32_t buffer = 0;
//     unsigned bufferedBits = 0;

//     for (unsigned i = 0; i < (bytesRead * 8) / bits; i++)
//     {
//         buffer = buffer | ((uint32_t(*resultsToPack) & mask) << bufferedBits);
//         bufferedBits += bits;

//         resultsToPack++;

//         if (bufferedBits == 32 || i == ((bytesRead * 8) / bits) - 1)
//         {
//             *resultBuffer = shuffle32(bits, buffer);
//             resultBuffer++;
//             buffer = 0;
//             bufferedBits = 0;
//         }
//     }
// }

// bool write_chunk(int fd, uint32_t bytesToRead, uint32_t *resultBuffer)
// {
//     int got = write(fd, resultBuffer, bytesToRead);
//     if (got != bytesToRead)
//     {
//         return false;
//     }
//     if (got <= 0){
//      throw std::invalid_argument("Write failure.");
//  }
//     return true;
// }