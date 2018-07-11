#include "thread.h"



namespace Thread{


  void Schedule::Lock(){
    int i=0;
    while (this->active.try_lock() == false){
      if (i > 1000){
        std::cerr << "Warn: Unexpected behaviour, something is blocking a schedule" << std::endl;
        i = 0;
      }

      i++;
    };
  };
  void Schedule::Unlock(){
    this->active.unlock();
  };

  void Schedule::Dispatch(Job job){
    this->Lock();
    this->jobs += 1;

    // Find an empty slot
    // Place the entry within said slot
    int length = this->data.capacity();
    for (int i=0; i<length; i++){
      if ( this->empty[i] == true ){
        this->data[i] = job;
        this->empty[i] = false;
        this->Unlock();
        return;
      }
    }

    // Expand the schedule size to fit
    // Place the entry at the first new index
    data.resize( length + 50 );
    empty.resize( data.capacity() );
    this->data[length] = job;
    this->empty[length] = false;
    this->Unlock();
    return;
  };
  JobResult Schedule::Search(int workerID){
    JobResult res;

    // Ensure no other thread is manipulating the Schedule
    this->Lock();

    // Search for an available job,
    // Start for the previous searches end point,
    // This will remove the advantage of as task being at the
    //   front of the queue
    unsigned int length = this->data.capacity();
    unsigned int i=this->lastFound+1;
    while (true){
      // Loop the search
      if (i>length){
        i -= length;
      }

      // A full search of the Schedule has been done
      // No results found, break search
      if (i == this->lastFound){
        break;
      }

      if (this->empty[i] == false){
        // Parse the information to the result
        res.found = true;
        res.result = this->data[i];

        // Remove this job from the queue
        this->lastFound = i;
        this->empty[i] = true;
        this->jobs -= 1;

        // Return and wrap up operation
        this->Unlock();
        return res;
      }

      i++;
    }

    res.found = false;
    return res;
  };
  unsigned int Schedule::Jobs(){
    return this->jobs;
  };





  void Worker::Initilize(unsigned int id, Schedule *unassignedHeap){
    this->id = id;
    this->anonymous = unassignedHeap;
    this->awake = false;
  };
  bool Worker::Wake(){
    if (this->awake == false){
      this->awake = true;

      std::thread (&Worker::Process, this).detach();
      return true;
    }else{
      return false;
    }
  };
  void Worker::Assign(Job task){
    this->work.Dispatch(task);

    // Ensure that the thread is awake to process the task
    this->Wake();
  };
  void Worker::Process(){
    JobResult res;

    // Repeat until no task is found
    while (true){
      // Find a job
      // First attempt to find any unassigned jobs
      res = this->anonymous->Search(this->id);
      // Then look though designated space
      if (res.found == false){
        res = this->work.Search(this->id);
      }

      if (res.found == true){
        res.result.ptr->workerID = this->id; // If the instance was anonymous, claim it
        res.result.ptr->Execute(res.result.cursor);
      }else{
        this->awake = false;
        return;
      }
    }
  }
  unsigned int Worker::Jobs(){
    return this->work.Jobs();
  }





  Pool::Pool(int threads){
    this->workers = threads;
    this->worker.resize(threads);

    // Assign Worker's default values
    for (int i=0; i<threads; i++){
      this->worker[i].Initilize(i, &this->anonymous);
    }
  }
  void Pool::Dispatch(Job task, bool targeted, unsigned int workerID){
    if (targeted == false){
      this->anonymous.Dispatch(task);

      // If there is a thread sleeping,
      // Wake it so it can take the anonymous task
      for (int i=0; i<this->workers; i++){
        // If the worker was woken,
        //   then stop
        if (this->worker[i].Wake() == true){
          break;
        }
      }
    }

    if (workerID > workers){
      std::cerr << "Error: Attempting to assign task to invalid worker" << std::endl;
      return;
    }

    this->worker[workerID].Assign(task);
  }
  bool Pool::Active(){
    for (int i=0; i<workers; i++){
      if (this->worker[i].Active() == true){
        return true;
      }
    }

    return false;
  }
  void Pool::WakeAll(){
    for (int i=0; i<this->workers; i++){
      this->worker[i].Wake();
    }
  }
  unsigned int Pool::Jobs(){
    unsigned int tally = 0;

    for (int i=0; i<this->workers; i++){
      tally += this->worker[i].Jobs();
    }

    return tally;
  };
  void Pool::Wedge(){
    bool active = true;

    while (active){
      active = this->Active();

      if (!active && this->Jobs() > 0){
        this->WakeAll();
        active = true;
      }
    }
  }


};
