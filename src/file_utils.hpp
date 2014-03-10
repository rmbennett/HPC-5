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


uint64_t shuffle64(unsigned bits, uint64_t x);

void unpack_blob(unsigned bits, uint32_t pixPerChunk, uint64_t *chunksRead, const uint64_t pRaw, float *pixBufStart, float **pixBufInsertPtr, float *pixBufEnd);

void unpack_blob32(unsigned bits, uint32_t pixPerChunk, uint64_t *chunksRead, const uint64_t pRaw, double *pixBufStart, double **pixBufInsertPtr, double *pixBufEnd);

void pack_blob(unsigned bits, uint32_t pixPerChunk, const float *pUnpacked, uint64_t *pRaw);

void pack_blob32(unsigned bits, uint32_t pixPerChunk, const double *pUnpacked, uint64_t *pRaw);

bool read_blob(int fd, uint64_t cbBlob, void *pBlob);

void write_blob(int fd, uint64_t cbBlob, const void *pBlob);
