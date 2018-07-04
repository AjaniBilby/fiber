#include "memory.h"

namespace Memory{
  void * Allocate(size_t space){
    #ifdef MemorySafe
      void *ptr;
      ptr = malloc(space);
      memset (ptr, 0, space);
      return ptr;
    #else
      return malloc(space);
    #endif
  };

  void UnAllocate(void *ptr){
    free(ptr);
  };

  void *Duplicate(void *destination, void *source, size_t space){
    return memcpy(destination, source, space);
  };
}