#pragma once

#include <chrono>
#include <thread>
#include <atomic>

#include "./instance.hpp"

namespace Thread{
  class Invoice{
    public:
      void Init();
      bool Attempt(int id);
      bool Assign(Instance *ptr, int cursor);
    private:
      std::atomic<bool> completed;
      std::atomic<bool> working;

      // Invoice details
      Instance *task;
      int start;

      void Process();
  };

  class Worker{
    public:
      std::atomic<bool> alive;

      void Init(int ID);
      bool Wake();
    private:
      int id;

      void Sleep();
      void Process();
  };

  #define MAX_QUEUE 1000U
  #define MAX_WORKERS 8U

  Invoice queue[ MAX_QUEUE ];
  Worker workers[ MAX_WORKERS ];

  void Wedge();
  void Dispatch(Instance *ptr, int cursor = 0);
  void Init();
}

#include "./thread.cpp"
