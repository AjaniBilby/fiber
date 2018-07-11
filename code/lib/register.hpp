#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <cmath>


#include "./hexidecimal.hpp"







union Handle {
  void *address;

  int8_t int8;
  uint8_t uint8;

  int16_t int16;
  uint16_t uint16;

  float float32;
  int32_t int32;
  uint32_t uint32;

  double float64;
  int64_t int64;
  uint64_t uint64;
};
enum RegisterMode: unsigned long {
  // Single Byte Operation
  uint8,
  int8,

  // 2 Byte Opeartions
  uint16,
  int16,

  // 4 Byte Operation
  float32,
  uint32,
  int32,

  // 8 Byte Operations
  float64,
  uint64,
  int64,
};
class Register{
  public:
    RegisterMode mode = RegisterMode::uint64;
    Handle value;

    unsigned long long int read();
    void write(unsigned long long int val);
    void Translate(RegisterMode toMode);
};
int GetRegisterID(std::string str);




bool ValidTypeSizing();


#include "./register.cpp"
