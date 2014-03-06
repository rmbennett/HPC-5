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

/*! Reverse the orders of bits if necessary
    \note This is laborious and a bit pointless. I'm sure it could be removed, or at least moved...
*/
// uint64_t shuffle64(unsigned bits, uint64_t x)
// {
//  if(bits==1){
//      x=((x&0x0101010101010101ull)<<7)
//          | ((x&0x0202020202020202ull)<<5)
//          | ((x&0x0404040404040404ull)<<3)
//          | ((x&0x0808080808080808ull)<<1)
//          | ((x&0x1010101010101010ull)>>1)
//          | ((x&0x2020202020202020ull)>>3)
//          | ((x&0x4040404040404040ull)>>5)
//          | ((x&0x8080808080808080ull)>>7);
//  }else if(bits==2){
//      x=((x&0x0303030303030303ull)<<6)
//          | ((x&0x0c0c0c0c0c0c0c0cull)<<2)
//          | ((x&0x3030303030303030ull)>>2)
//          | ((x&0xc0c0c0c0c0c0c0c0ull)>>6);
//  }else if(bits==4){
//      x=((x&0x0f0f0f0f0f0f0f0full)<<4)
//          | ((x&0xf0f0f0f0f0f0f0f0ull)>>4);
//  }
//  return x;
// }

/*! Take data packed into incoming format, and exand to one integer per pixel */
// void unpack_blob(unsigned w, unsigned h, unsigned bits, const uint64_t *pRaw, uint32_t *pUnpacked)
// {
//  uint64_t buffer=0;
//  unsigned bufferedBits=0;

//  const uint64_t MASK=0xFFFFFFFFFFFFFFFFULL>>(64-bits);

//  for(unsigned i=0;i<w*h;i++){
//      if(bufferedBits==0){
//          buffer=shuffle64(bits, *pRaw++);
//          bufferedBits=64;
//      }

//      pUnpacked[i]=buffer&MASK;
//      buffer=buffer>>bits;
//      bufferedBits-=bits;
//  }

//  assert(bufferedBits==0);
// }

/*! Go back from one integer per pixel to packed format for output. */
// void pack_blob(unsigned w, unsigned h, unsigned bits, const uint32_t *pUnpacked, uint64_t *pRaw)
// {
//  uint64_t buffer=0;
//  unsigned bufferedBits=0;

//  const uint64_t MASK=0xFFFFFFFFFFFFFFFFULL>>(64-bits);

//  for(unsigned i=0;i<w*h;i++){
//      buffer=buffer | (uint64_t(pUnpacked[i]&MASK)<< bufferedBits);
//      bufferedBits+=bits;

//      if(bufferedBits==64){
//          *pRaw++ = shuffle64(bits, buffer);
//          buffer=0;
//          bufferedBits=0;
//      }
//  }

//  assert(bufferedBits==0);
// }

// bool read_blob(int fd, uint64_t cbBlob, void *pBlob)
// {
//  uint8_t *pBytes=(uint8_t*)pBlob;

//  uint64_t done=0;
//  while(done<cbBlob){
//      int todo=(int)std::min(uint64_t(1)<<30, cbBlob-done);

//      int got=read(fd, pBytes+done, todo);
//      if(got==0 && done==0)
//          return false;   // end of file
//      if(got<=0)
//          throw std::invalid_argument("Read failure.");
//      done+=got;
//  }

//  return true;
// }

// void write_blob(int fd, uint64_t cbBlob, const void *pBlob)
// {
//  const uint8_t *pBytes=(const uint8_t*)pBlob;

//  uint64_t done=0;
//  while(done<cbBlob){
//      int todo=(int)std::min(uint64_t(1)<<30, cbBlob-done);

//      int got=write(fd, pBytes+done, todo);
//      if(got<=0)
//          throw std::invalid_argument("Write failure.");
//      done+=got;
//  }
// }

bool read_chunk(int fd, uint32_t bytesToRead, uint32_t *readBuffer)
{

    int got = read(fd, readBuffer, bytesToRead);
    if (got != bytesToRead)
        return false;   // end of file
    if (got <= 0)
        throw std::invalid_argument("Read failure.");

    return true;

}

void unpack_chunk(uint32_t bytesToRead, unsigned bits, uint64_t *chunksRead, unsigned mask, uint32_t *readBuffer, float *pixBufStart, float **pixBufInsertPtr, float *pixBufEnd)
{
    for (int i = 0; i < (bytesToRead * 8) / bits; i++)
    {
    	fprintf(stderr, "Index: %d\n", (i * bits) / (8 * sizeof(uint32_t)));
        **pixBufInsertPtr = (float)(readBuffer[(i * bits) / (8 * sizeof(uint32_t))] >> (i * bits) & mask);
        (*pixBufInsertPtr)++;
        if (*pixBufInsertPtr == pixBufEnd)
        {
            *pixBufInsertPtr = pixBufStart;
        }
    }
    (*chunksRead)++;
}

void pack_chunk(uint32_t bytesRead, unsigned bits, unsigned mask, uint32_t *resultBuffer, float *resultsToPack)
{
    uint32_t buffer = 0;
    unsigned bufferedBits = 0;

    for (unsigned i = 0; i < (bytesRead * 8) / bits; i++)
    {
    	fprintf(stderr, "%d\n", uint32_t(*resultsToPack));
        buffer = buffer | ((uint32_t(*resultsToPack) & mask) << bufferedBits);
        bufferedBits += bits;

        resultsToPack++;

        if (bufferedBits == 32 || i == ((bytesRead * 8) / bits) - 1)
        {
        	fprintf(stderr, "Writing to resbuf\n");
            *resultBuffer = buffer;
            resultBuffer++;
            buffer = 0;
            bufferedBits = 0;
        }
    }
}

bool write_chunk(int fd, uint32_t bytesToRead, uint32_t *resultBuffer)
{
    int got = write(fd, resultBuffer, bytesToRead);
    if (got != bytesToRead)
    {
        return false;
    }
    if (got <= 0){
		throw std::invalid_argument("Write failure.");
	}
    return true;
}