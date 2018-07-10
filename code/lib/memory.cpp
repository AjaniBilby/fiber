#include "memory.h"

namespace Memory{
  void * Allocate(size_t space){
    #ifdef MemorySafe
      void *ptr;
      ptr = malloc(space);

      // Fill section with 0
      char *a = static_cast<char*>(ptr);
      while (space > 0){
        a = 0;

        space -= 1;
        a += 1;
      }

      return ptr;
    #else
      return malloc(space);
    #endif
  };

  void UnAllocate(void *ptr){
    free(ptr);
  };

  void Duplicate(void *destination, void *source, size_t space){
    char *a = static_cast<char*>(source);
    char *b = static_cast<char*>(destination);

    // Copy memory byte per byte
    while (space > 0){
      b = a;

      // Shift reference
      space -= 1;
      a += 1;
      b += 1;
    }
  };
}
