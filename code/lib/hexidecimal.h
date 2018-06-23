#ifndef Hexidecimal_H
#define Hexidecimal_H

/*
  Purpose:
    To convert hexidecimal numbers of which are encoded as strings,
    into signed long (64bit) integers
*/

#include <iostream>
#include <sstream>
#include <string>

namespace Hexidecimal {
  long int convert(std::string input);
}


#endif