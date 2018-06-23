#include "hexidecimal.h"

namespace Hexidecimal {
  long int convert(std::string input){
    unsigned long int x;
    std::stringstream temp;
    temp << std::hex << input;

    temp >> x;
    return static_cast<long int>(x);
  }
}