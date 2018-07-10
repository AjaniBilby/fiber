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
#include <cmath>

#include "register.h"
#include "segregate.h"





enum Commands {
  invalid,       // Unexpected Zero
  blank,         // Intensionally empty space

  // Memory movement
  memory,
  push,
  pull,

  // Std io
  standardStream,
  
  // Register actions
  translate,
  mode,
  copy,
  move,
  set,

  // Calcuations
  bitwise,
  math,

  // Comparators
  longCompare,
  compare,

  // Scheduling
  // Suspend,

  // Flow Control
    Continue,
    Break,
    GOTO,
    stop,
    END,

    // If statement
    IF,
    ELSE,

    Loop
};

enum MathOpperation {
  add,
  subtract,
  divide,
  multiply,
  modulus,
  exponent
};

enum Comparason{
  equal,
  less,
  greater
};
enum BitOperator{
  AND,
  OR,
  XOR,
  LeftShift,
  RightShift,
  NOT
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

    unsigned int resultSize;
    unsigned int localSize;

    std::vector<Function *> children;
    std::vector<Action> code;

    Function(std::string name, unsigned int local, unsigned int result);
    bool Parse(Segregate::StrCommands source);
    int GetChildsID(std::string str); // Get the function ID number of a child
  private:
    bool Interpret(Segregate::StrCommands source);
    bool SimplifyIF();
    bool SimplifyLoop();
    bool CheckBlockEndPoints();
};


#include "function.cpp"
#endif