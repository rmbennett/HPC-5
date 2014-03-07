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

// uint64_t shuffle64(unsigned, uint64_t);
// void unpack_blob(unsigned, unsigned, unsigned, const uint64_t*, uint32_t*);
// void pack_blob(unsigned, unsigned, unsigned, const uint32_t*, uint64_t*);
// bool read_blob(int, uint64_t, void*);
// void write_blob(int, uint64_t, const void*);

bool read_chunk(int, uint32_t, uint32_t*);
uint32_t shuffle32(unsigned bits, uint32_t x);
void unpack_blob(unsigned w, unsigned h, uint32_t bytesToRead, unsigned bits, uint64_t *chunksRead, uint32_t *readBuffer, float *pixBufStart, float **pixBufInsertPtr, float *pixBufEnd);
// void unpack_chunk(uint32_t, unsigned, uint64_t*, uint32_t, uint32_t*, float*, float**, float*);
// void pack_blob(unsigned w, unsigned h, uint32_t bytesToRead, unsigned bits, float *resultsToPack, uint32_t *resultBuffer);
void pack_chunk(uint32_t, unsigned, unsigned, uint32_t*, float*);
bool write_chunk(int, uint32_t, uint32_t*);
