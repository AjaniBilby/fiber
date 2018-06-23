#ifndef Function_H
#define Function_H

/*
  Purpose:
    To take tokens and simplify them into a form quicker to be interperted
    These function also execute their code as part of their code,
    However this execution is controlled by individual instances,
    of which supply and hold register information, as well as linking information
    into the stack tree.
*/

#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <cmath>

#include "register.cpp"
#include "segregate.cpp"





enum Commands {
  invalid,       // Keep invalid and stop seperate
  stop,          // so that unset commands can still be detected
  set,           // Change the value of a register
  memory,
  standardStream,
  push,
  pull,
  mode,
  translate,
  math,
  copy,
};

enum MathOpperation {
  add,
  subtract,
  divide,
  multiply,
  modulus,
  exponent
};

struct Action {
  Commands command;                          // Command enum ID
  std::vector<unsigned long long int> param; // Parameters encoded as ints
  int line;
};






class Function{
  public:
    Function *parent;
    std::string name;
    int size;

    std::vector<Function *> children;
    std::vector<Action> code;

    Function(std::string name, int size);
    bool Parse(Segregate::StrCommands source);
    int GetChildsID(std::string str); // Get the function ID number of a child
  private:
};




bool ValidTypeSizing();

#endif