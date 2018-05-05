#include <iostream>
#include <vector>
#include <thread>
#include <chrono>

// typedef std::chrono::high_resolution_clock Time;  [Profile-Timing]
// Time execution_start;                             [Profile-Timing]
// Time execution_end;                               [Profile-Timing]
// std::chrono::duration<float> fsec;                [Profile-Timing]


namespace Thread{
  unsigned int MAX_STACK = 1000;
  unsigned int MAX_WORKERS = 4;



  class Task{
    public:
      // Request details
      unsigned int codePos   = 0;
      unsigned char *memPos  = 0;

      // Thread specific task
      bool target            = false;
      unsigned int to        = 0;

      // Who created this request
      unsigned int from      = 0;

      bool completed         = true;
      bool active            = false;

      // The constructor will need to be executed manually since this class will purely exist in a vector
      void Initilize(){
        this->target    = false;
        this->completed = true;
        this->active    = false;
      }

      // Check if the requester can forfill the task
      //   If so, then mark it as in progress so the thread can do it
      bool Attempt(unsigned int WorkerID){
        // See if the caller meets the requirements
        if (this->target){
          if (this->to != WorkerID){
            return false;
          }
        }

        // See if the task is in progress
        if (this->active == false){
          return false;
        }

        // Assign the thread to doing the task
        this->active = false;

        // Allow future threads to stop befor checking the atomic later
        this->target = true;
        this->to = WorkerID;

        return true;
      };

      // Mark the task as done
      void Complete(){
        this->completed = true;

        return;
      };

      // Presume that the task has been reassigned externally
      //   Setup internals for a thread to forfill it later
      void Pend(){
        this->active = false;
        this->completed = false;
      };

    private:

  };
  // Generate Processing Stack
  std::vector<Task> stack;





  struct Job{
    bool found = false;
    unsigned int taskID;
  };

  class Worker{
    public:
      unsigned int id     = 0;
      
      unsigned int taskID = 0;
      bool active         = false;

      // The constructor will need to be executed manually since this class will purely exist in a vector
      void Initilize(int givenId){
        this-> id     = givenId;
        this->taskID  = 0;
        this-> active = false;
      }

      // See if the worker has fallen asleep,
      //   If so awaken it
      //   Return if the thread actually did wake up from this event
      bool Wake(){
        std::cout << "Thread["<<this->id << "]: Waking..." << std::endl;

        if (this->active == false){
          std::thread handle (&Worker::Process, this);
          handle.detach();

          return true;
        }else{
          return false;
        }
      };

      void Collapse();
    private:
      Job Seek(){
        std::cout<< "SEEKING" << std::endl;

        int i = taskID + 1;
        int j = 0;
        Job result;

        // Loop all the way around the stack searching for a job.
        // Start from the previous job position and look left
        //   This will allow the system to not miss any jobs
        //   (Note new jobs are assigned by filling old spots from 0 to max index)
        while (i != taskID){
          for (j=0; j<MAX_STACK; j++){
            result.found = stack[j].Attempt(this->id);

            if (result.found == true){
              result.taskID = j;
              return result;
            }
          }

          // Step
          i++;
          if (i > MAX_STACK){
            i -= MAX_STACK;
          }
        }

        return result;
      };

      void Process(){
        this->active = true;
        Job proposal;

        while (this->active == true){
          // Find a job
          proposal = this->Seek();

          if (proposal.found == false){
            this->Collapse();
            return;
          }else{

            // Operate on the task
            std::cout << "Found Task for thread " << this->id << " " << proposal.taskID << std::endl;

            stack[proposal.taskID].Complete();
          }
        }
      };
  };
  // Declare work force
  std::vector<Worker> workers;

  void Worker::Collapse(){
    this->active = false;
        std::cout << "Thread[" << this->id << "]: Sleep..." << std::endl;

        // See if this is the last thread to close
        for (int i=0; i<MAX_WORKERS; i++){
          if (workers[i].active){
            return;
          }
        }

        // If so, then mark now as the end time
        // execution_end = Time::now(); [Profile-Timing]
  }




  void Dispatch(unsigned int codePos, unsigned char* memPos, bool target, unsigned int to, unsigned int from){
    for (int i=0; i<MAX_STACK; i++){
      if (stack[i].completed){
        // Incase another thread is blocking this slot while it fills it
        if (stack[i].active == true){
          continue;
        }

        stack[i].active = true; // Stop another thread taking this slot

        // Copy the task data to the task slot
        stack[i].codePos = codePos;
        stack[i].memPos = memPos;
        stack[i].target = target;
        stack[i].to = to;
        stack[i].from = from;

        stack[i].active = false; // Alow a thread to start for on the slot
        stack[i].completed = false;

        if (target == true){ // Ensure that the target is awake
          workers[to].Wake();
        }else{       // If there is no target, but a worker is sleeping, wake it
          for (int j=0; j<MAX_WORKERS; j++){
            // If this wake action did wake it, and it wasn't already active.
            if (workers[j].Wake()){
              return;
            }
          }
        }

        return;
      }
    }

    std::cerr << "Stack overflow, to many async requests" << std::endl << "  more than (" << MAX_STACK << ")" << std::endl;
    return;
  };

  float KeepAlive(){
    while (true){
      // If there are still elements in the stack to be done, then hold processing
      for (int i=0; i<MAX_STACK; i++){
        if (stack[i].completed == false){
          std::cout<< "Remaining tasks exist " << i << std::endl;
          std::this_thread::sleep_for(std::chrono::milliseconds(500));
          continue;
        }
      }

      // fsec fs = execution_end - execution_start;  [Profile-Timing]
      return 0;
    }
  }


  void Initilize(){
    stack.reserve(MAX_STACK);
    workers.reserve(MAX_WORKERS);

    for (int i=0; i<MAX_WORKERS; i++){
      workers[i].Initilize(i);
    }
    for (int i=0; i<MAX_STACK; i++){
      stack[i].Initilize();
    }

    // execution_start = Time::now();  [Profile-Timing]
    return;
  }
}