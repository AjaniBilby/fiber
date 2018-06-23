#ifndef THREAD_H
#define THREAD_H

#include <iostream>
#include <thread>
#include <vector>
#include <mutex>


#include "instance.cpp"


namespace Thread{


  struct Job{
    Instance *ptr;
    unsigned int cursor;
  };

  struct JobResult {
    bool found;
    Job result;
  };

  class Schedule{
    private:
      std::vector<Job> data;
      std::vector<bool> empty;

      std::mutex active;
      unsigned int lastFound = 0;
      unsigned int jobs = 0;
      void Lock();
      void Unlock();


    public:
      void Dispatch(Job job);
      JobResult Search(int workerID);
      unsigned int Jobs();
  };

  class Worker{
    private:
      bool awake = false;
      void Process();

      Schedule work;
      Schedule *anonymous;
      unsigned int id = 0;
    public:
      void Initilize(unsigned int id, Schedule *unassignedHeap);
      bool Wake();
      void Assign(Job task);
      bool Active(){
        return this->awake;
      }

      unsigned int Jobs();
  };



  class Pool{
    private:
      Schedule anonymous;

      std::vector<Worker> worker;
      unsigned int workers;

      void WakeAll();
    public:
      Pool(int threads);
      void Dispatch(Job task, bool targeted, unsigned int workerID);
      void Wedge();
      bool Active();

      unsigned int Jobs();
  };

  
};


#endif