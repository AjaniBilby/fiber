#ifndef Thread_H
#define Thread_H

#include <iostream>
#include <atomic>
#include <vector>
#include <thread>
#include <chrono>

unsigned int MAX_STACK = 1000;
unsigned int MAX_WORKERS = 4;



class Task{
  public:
    // Request details
    unsigned int codePos;
    unsigned char *memPos;

    // Thread specific task
    bool target;
    unsigned int to;

    // Who created this request
    unsigned int from;

    bool completed;
    bool active;

    // The constructor will need to be executed manually since this class will purely exist in a vector
    void Initilize();

    // Check if the requester can forfill the task
    //   If so, then mark it as in progress so the thread can do it
    bool Attempt(unsigned int WorkerID);

    // Mark the task as done
    void Complete();

    // Presume that the task has been reassigned externally
    //   Setup internals for a thread to forfill it later
    void Pend();
  private:

};
// Generate Processing Stack
std::vector<Task> stack;





struct Job{
  bool found;
  unsigned int taskID;
};

class Worker{
  public:
    unsigned int id;
    
    unsigned int taskID;
    bool active;

    // The constructor will need to be executed manually since this class will purely exist in a vector
    void Initilize(int givenId);

    // See if the worker has fallen asleep,
    //   If so awaken it
    //   Return if the thread actually did wake up from this event
    bool Wake();
  private:
    Job Seek();
    void Collapse();
    void Process();
};
// Declare work force
std::vector<Worker> workers;







void Dispatch(unsigned int codePos, unsigned char* memPos, bool target, unsigned int to, unsigned int from);
void KeepAlive();

#endif Thread_H