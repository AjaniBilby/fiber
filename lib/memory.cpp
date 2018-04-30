#include <stdlib.h>
#include <iostream>

namespace memory{

  unsigned char* Allocate(uint64_t amount){
    return (unsigned char*) malloc(amount+1);
  };

  void Unallocate(unsigned char* ptr, uint64_t amount){
    free(ptr);

    return;
  };

}