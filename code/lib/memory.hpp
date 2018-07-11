#pragma once

#include <memory>

namespace Memory{
  void *Allocate(size_t space);
  void UnAllocate(void *ptr);

  void Duplicate(void *destination, void *source, size_t space);
};

#include "./memory.cpp"
