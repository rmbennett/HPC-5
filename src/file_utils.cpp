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