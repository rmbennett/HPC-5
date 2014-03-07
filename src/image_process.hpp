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

void erodeChunk(unsigned w, unsigned h, int levels, uint32_t pixPerChunk, uint32_t chunksPerLine, uint64_t *chunksProcessed, float *pixBufStart, float *pixCalculate, float *pixBufEnd, float *processedResultsBuffer);
void dilateChunk(unsigned w, unsigned h, int levels, uint32_t pixPerChunk, uint32_t chunksPerLine, uint64_t *chunksProcessed, float *pixBufStart, float *pixCalculate, float *pixBufEnd, float *processedResultsBuffer);
bool validPixel(unsigned w, unsigned h, unsigned x, unsigned y, int dx, int dy);
float getPixel(unsigned w, unsigned h, unsigned x, unsigned y, int dx, int dy, float* memStart, float *pixPtr, float *memEnd);
void calculateChunkXY(unsigned w, unsigned h, unsigned *x, unsigned *y, uint64_t chunksProcessed, uint32_t chunksPerLine, uint32_t pixPerChunk);
bool processStreamChunk(unsigned w, unsigned h, int levels, uint32_t pixPerChunk, uint32_t chunksPerLine, uint32_t totalChunks, uint64_t *originalImgChunksProcessed, float *pixBufStart, float **pixCalculate, float *pixBufEnd, uint64_t *irChunksProcessed, float *irStart, float **irInsert, float **irCalculate, float *irEnd, float *finalResult);

int mapIJKtoDX(int i, int j, int k, int levels);
int mapIJKtoDY(int i, int j, int k, int levels);

