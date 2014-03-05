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

uint64_t shuffle64(unsigned, uint64_t);
void unpack_blob(unsigned, unsigned, unsigned, const uint64_t*, uint32_t*);
void pack_blob(unsigned, unsigned, unsigned, const uint32_t*, uint64_t*);
bool read_blob(int, uint64_t, void*);
void write_blob(int, uint64_t, const void*);

