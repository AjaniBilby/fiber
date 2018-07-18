#include "hexidecimal.hpp"

namespace Hexidecimal {
  unsigned long long convert(std::string input){
    unsigned long long x;
    std::stringstream temp;
    temp << std::hex << input;

    temp >> x;
    return x;
  };
}
