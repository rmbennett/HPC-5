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

uint32_t vmin(uint32_t, uint32_t);
uint32_t vmin(uint32_t, uint32_t, uint32_t);
uint32_t vmin(uint32_t, uint32_t, uint32_t, uint32_t);
uint32_t vmin(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);

void erode(unsigned, unsigned, const std::vector<uint32_t>&, std::vector<uint32_t>&);

uint32_t vmax(uint32_t, uint32_t);
uint32_t vmax(uint32_t, uint32_t, uint32_t);
uint32_t vmax(uint32_t, uint32_t, uint32_t, uint32_t);
uint32_t vmax(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);

void dilate(unsigned, unsigned, const std::vector<uint32_t>&, std::vector<uint32_t>&);

void process(int, unsigned, unsigned, unsigned, std::vector<uint32_t>&);
