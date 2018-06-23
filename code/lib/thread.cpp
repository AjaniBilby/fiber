#include "thread.h"

namespace Thread{

  // PUBLIC
  void Invoice::Init(){
    this->completed = true;
  };
  bool Invoice::Attempt(int id){
    if (this->completed){
      return false;
    }
    if (this->working){
      return false;
    }

    if (this->task->assigned == true){
      if (this->task->workerID != id){
        return false;
      }
    }else{  // If this task is not asssigned, then make the requester own it
      this->task->assigned = true;
      this->task->workerID = id;
    }

    this->Process();
    return true;
  };
  bool Invoice::Assign(Instance *ptr, int cursor){
    // Do not overwrite an awaiting/inprogress task
    if (this->completed == false){
      return false;
    }

    // Prevent other's from editing
    this->completed = true;
    this->working = true;

    this->task = ptr;
    this->start = cursor;

    // Make invoice available
    this->completed = false;
    this->working = false;

    // Wake the worker required if it is asleep
    if (ptr->assigned == true){
      workers[ptr->workerID].Wake();
    }else{
      // Wake a single sleeping worker
      // So it can self assign the task
      for (int i=0; i<MAX_WORKERS; i++){
        if (workers[i].Wake()){
          return true;
        }
      }
    }

    return true;
  }
  // PRIVATE
  void Invoice::Process(){
    this->working = true;
    this->task->Execute(this->start);
    this->completed = true;

    return;
  };





  // PUBLIC
  void Worker::Init(int ID){
    this->alive = false;
    this->id = ID;
  };
  bool Worker::Wake(){
    if (this->alive == false){
      this->alive = true;

      std::thread (&Worker::Process, this).detach();
      return true;
    }else{
      return false;
    }
  }

  // PRIVATE
  void Worker::Sleep(){
    this->alive = false;
    
    // std::cout << "Thread[" << this->id << "]: Hibernating..." << std::endl;

    // Only pass this block if all workers are asleep
    // for (int i=0; i<MAX_WORKERS){
    //   if (workers[i].alive == true){
    //     return;
    //   }
    // }
  }
  void Worker::Process(){
    int last = MAX_QUEUE-1;
    int i = 0;

    // std::cout << "Thread[" << this->id << "]: Now Active" << std::endl;

    while (true){

      // Reset the loop scan end point
      if (queue[i].Attempt( this->id ) == true){
        last = i;
      }else if (last == i){
        // If this element has been attempted and was re-attempted,
        // presume there is nothing in the queue for this worker
        break;
      }

      // Looping search
      i = (i + 1) % MAX_QUEUE;
    }

    this->Sleep();
  }





  void Wedge(){
    repeat:
    // Block passage if a worker is active
    for (int i=0; i<MAX_WORKERS; i++){
      if (workers[i].alive == true){
        std::this_thread::sleep_for( std::chrono::milliseconds(100) );
        goto repeat;
      }
    }
  }

  void Dispatch(Instance *ptr, int cursor){
    // Search for an empty space
    for (int i=0; i<MAX_QUEUE; i++){
      if (queue[i].Assign(ptr, cursor) == true){
        
        return;
      }
    }
  }

  void Init(){
    for (int i=0; i<MAX_QUEUE; i++){
      queue[i].Init();
    }
    for (int i=0; i<MAX_WORKERS; i++){
      workers[i].Init(i);
    }
  }

}