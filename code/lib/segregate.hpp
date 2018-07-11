#pragma once

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
  struct StrCommand{
    unsigned long line;
    ParamArray param;
  };
  typedef std::vector< StrCommand > StrCommands;

  bool IsParamBreak(char val);
  bool IsLineBreak(char val);

  std::string RemoveCarrageReturn(std::string str);

  ParamArray Parameterize(std::string str);
  StrCommands Fragment(std::string str);
}


#include "./segregate.cpp"
