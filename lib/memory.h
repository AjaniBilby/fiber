#ifndef memory
#define memory
#include <iostream>

namespace memory{
  unsigned char* Allocate(uint64_t amount);
  void Unallocate(unsigned char* ptr, uint64_t amount);
}

#endif