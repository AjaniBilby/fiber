#pragma once

/*
  Purpose:

*/


#include "./function.hpp"
#include "./thread.hpp"
#include "./memory.hpp"

#include <vector>
#include <string>

class Instance{
  public:
    int cursor = -1;
    Register handle[12];
    void Execute(unsigned long cursor);

    bool assigned = false;
    int workerID; // Which 'thread' is this instance locked to

    Instance(Function *reference, Instance *caller, Thread::Pool *pool);
    bool IsChild(Instance *ptr);

    int returnPos;    // The point to go back to once completed
    int yeildPos;     // The point to go back to once a value has been found

    unsigned long long GetLocalSpace();

    /*
      Prevent's instance from futther execution and returning
    */
    void Destory();

    bool HasDestoryed();

    // Stop multiple sensitive operations happening at once
    std::recursive_mutex sensitive;
  private:
    Instance *parent;
    Function *ref;
    Thread::Pool *pool;

    unsigned long long localMemory; // Points to the local memory segment

    bool connected; // Whether it's parent has cut execution

    std::vector<Instance *> child;

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
    unsigned long CmdInstance(Action *act, unsigned long cursor);
    void CmdLocal(Action *act);
};


#include "./instance.cpp"
