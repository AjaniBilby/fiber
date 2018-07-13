#pragma once

/*
  Purpose:

*/


#include "./function.hpp"
#include "./instance.hpp"
#include "./thread.hpp"
#include "./memory.hpp"

#include <vector>
#include <string>

class Instance{
  public:
    int cursor = -1;
    Register handle[12];
    void Execute(int cursor);

    bool assigned = false;
    int workerID; // Which 'thread' is this instance locked to

    Instance(Function *reference, Instance *caller);
    bool IsChild(Instance *ptr);

  private:
    Instance *parent;
    Function *ref;

    std::vector<Instance *> children;

    int returnPos;    // The point to go back to once completed
    int yeildPos;     // The point to go back to once a value has been found

    void CmdSet(Action *act);
    void CmdSS(Action *act);
    void CmdMem(Action *act);
    void CmdPush(Action *act);
    void CmdPull(Action *act);
    void CmdMode(Action *act);
    void CmdTranslate(Action *act);
    void CmdMath(Action *act);
    void CmdCopy(Action *act);
    void CmdMove(Action *act);
    void CmdComp(Action *act);
    void CmdLComp(Action *act);
    void CmdBit(Action *act);
};


#include "./instance.cpp"
