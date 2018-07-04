#ifndef Instance_H
#define Instance_H

/*
  Purpose:
    
*/


#include "function.cpp"
#include "thread.cpp"

#include <vector>
#include <string>

class Instance{
  public:
    int cursor = -1;
    Register handle[12];
    void Execute(int cursor);

    bool assigned = false;
    int workerID; // Which 'thread' is this instance locked to

    Instance *parent; // Reference to the caller's position
    Function *ref;    // Reference to the byte code
    int returnPos;    // The point to go back to once completed
    int yeildPos;     // The point to go back to once a value has been found

    Instance(Function *reference, Instance *caller);

  private:
    bool relativeMode = false;
    Function *reference;

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

#endif