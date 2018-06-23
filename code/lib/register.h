#ifndef REGISTER_H
#define REGISTER_H

#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <cmath>


#include "hexidecimal.cpp"







union Handle {
  Handle *address;

  char int8;
  unsigned char uint8;

  short int int16;
  unsigned short int uint16;

  float float32;
  long int int32;
  unsigned long int uint32;

  double float64;
  long long int int64;
  unsigned long long int uint64;
};
enum RegisterMode: unsigned long long {
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





// For converting between address types
union Address {
  char *c;
  Handle *h;
};


#endif