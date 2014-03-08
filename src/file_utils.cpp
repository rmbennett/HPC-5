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

bool read_chunk(int fd, uint32_t bytesToRead, uint32_t *readBuffer)
{

    int got = read(fd, readBuffer, bytesToRead);
    if (got != bytesToRead)
        return false;   // end of file
    if (got <= 0)
        throw std::invalid_argument("Read failure.");

    return true;

}

uint32_t shuffle32(unsigned bits, uint32_t x)
{
	if(bits==1){
		x=((x&0x01010101ul)<<7)
			| ((x&0x02020202ul)<<5)
			| ((x&0x04040404ul)<<3)
			| ((x&0x08080808ul)<<1)
			| ((x&0x10101010ul)>>1)
			| ((x&0x20202020ul)>>3)
			| ((x&0x40404040ul)>>5)
			| ((x&0x80808080ul)>>7);
	}else if(bits==2){
		x=((x&0x03030303ull)<<6)
			| ((x&0x0c0c0c0cul)<<2)
			| ((x&0x30303030ul)>>2)
			| ((x&0xc0c0c0c0ul)>>6);
	}else if(bits==4){
		x=((x&0x0f0f0f0ful)<<4)
			| ((x&0xf0f0f0f0ul)>>4);
	}
	return x;
}

/*! Take data packed into incoming format, and exand to one integer per pixel */
void unpack_blob(unsigned w, unsigned h, uint32_t bytesToRead, unsigned bits, uint64_t *chunksRead, uint32_t *readBuffer, float *pixBufStart, float **pixBufInsertPtr, float *pixBufEnd)
{
	uint32_t buffer=0;
	unsigned bufferedBits=0;
	
	const uint32_t MASK=0xFFFFFFFFUL>>(32-bits);
	
	for(unsigned i=0;i<(bytesToRead * 8) / bits;i++){
		if(bufferedBits==0){
			buffer=shuffle32(bits, *readBuffer++);
			bufferedBits=32;
		}

		**pixBufInsertPtr=(float)(buffer&MASK);
		fprintf(stderr, "unpacked %f\n", **pixBufInsertPtr);
		sleep(1);
		(*pixBufInsertPtr)++;
		if ((*pixBufInsertPtr) >= pixBufEnd)
		{
			// fprintf(stderr, "Wrapping from %d\n", *pixBufInsertPtr);
			*pixBufInsertPtr = pixBufStart + ((*pixBufInsertPtr - pixBufEnd)/sizeof(float));
			// fprintf(stderr, "To %d, chunksRead %d, memstart %d memend %d\n", *pixBufInsertPtr, *chunksRead, pixBufStart, pixBufEnd);
		}
		buffer=buffer>>bits;
		bufferedBits-=bits;
	}
	
	(*chunksRead)++;

}


void pack_chunk(uint32_t bytesRead, unsigned bits, unsigned mask, uint32_t *resultBuffer, float *resultsToPack)
{
    uint32_t buffer = 0;
    unsigned bufferedBits = 0;

    for (unsigned i = 0; i < (bytesRead * 8) / bits; i++)
    {
        buffer = buffer | ((uint32_t(*resultsToPack) & mask) << bufferedBits);
        bufferedBits += bits;

        resultsToPack++;

        if (bufferedBits == 32 || i == ((bytesRead * 8) / bits) - 1)
        {
            *resultBuffer = shuffle32(bits, buffer);
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