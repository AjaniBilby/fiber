#ifndef Segregate_H
#define Segregate_H

#include <vector>
#include <string>

/*
  Purpose:
    To to take a whole file as a string,
    and break it up into individual words
*/

namespace Segregate {
  // Module types
  typedef std::vector< std::string > ParamArray;
  typedef std::vector< ParamArray > StrCommands;

  ParamArray Parameterize(std::string str);
  StrCommands Fragment(std::string str);
}


#include "segregate.cpp"
#endif