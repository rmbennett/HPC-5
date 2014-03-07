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

void erodeChunk(unsigned w, unsigned h, int levels, uint32_t pixPerChunk, uint32_t chunksPerLine, uint64_t *chunksProcessed, uint64_t chunksRead, float *pixBufStart, float *pixCalculate, float *pixBufEnd, float *processedResultsBuffer, bool doneFirst);
void dilateChunk(unsigned w, unsigned h, int levels, uint32_t pixPerChunk, uint32_t chunksPerLine, uint64_t *chunksProcessed, uint64_t chunksRead, float *pixBufStart, float *pixCalculate, float *pixBufEnd, float *processedResultsBuffer, bool doneFirst);
bool validPixel(unsigned w, unsigned h, unsigned x, unsigned y, int dx, int dy);
float getPixel(unsigned w, unsigned h, unsigned x, unsigned y, int dx, int dy, float* memStart, float *pixPtr, float *memEnd);
void calculateChunkXY(unsigned w, unsigned h, unsigned *x, unsigned *y, uint64_t chunksProcessed, uint32_t chunksPerLine, uint32_t pixPerChunk);
void processStreamChunk(unsigned w, unsigned h, int levels, uint32_t pixPerChunk, uint32_t chunksPerLine, uint64_t *chunksProcessed, uint64_t chunksRead, float *pixBufStart, float **pixCalculate, float *pixBufEnd, float *processedResultsBuffer);

int mapIJKtoDX(int i, int j, int k, int levels);
int mapIJKtoDY(int i, int j, int k, int levels);

