#include "./thread.hpp"




Thread::Worker::Worker(size_t wID, EventLoop::Schedule* wPool, void* tPool){
	this->workerID = wID;
	this->workPool = wPool;               // Receive unassigned work pool
	this->threadPool = tPool;
	this->awake = false;

	if (this->queue.size() != DEFAULT_QUEUE_LENGTH){
		std::cerr << "Error: Worker queue failed to initilize" << std::endl;
	}
};

void Thread::Worker::Issue(EventLoop::Task task){
	this->queue.Issue(task);

	// Ensure the worker is away to receive the task
	this->Wake();
	return;
};

void Thread::Worker::IssueToWorkPool(EventLoop::Task task){
	// Issue via the pool's system to it can wake sleeping workers
	reinterpret_cast<Thread::Pool*>(this->threadPool)->Issue(task);
};

void Thread::Worker::Wake(){
	// Another process is already waking this worker
	this->changeState.lock();

	// The thread is already awake
	if (this->awake == true){
		this->changeState.unlock();
		return;
	}

	// Revive the thread
	this->awake = true;
	std::thread (&Thread::Worker::Process, this).detach();

	// Revive complete
	this->changeState.unlock();
	return;
};


bool Thread::Worker::HasTasks(){
	return this->queue.HasTasks();
};


void Thread::Worker::Process(){
	EventLoop::Task task;
	bool found = true;
	while (found){
		found = false;

		#if (THREAD_OWN_TASK_PRIORITY)
			task = this->queue.Find();
			if (task.empty == true){
				task = this->workPool->Find();

				if (task.empty == false){
					// Claim the new work as it's own
					reinterpret_cast<Instance*>(task.reference)->owner = this;
				}else{
					// No task in unclaimed, or claimed work loads
					break;
				}
			}
		#else
			task = this->workPool->Find();
			if (task.empty == false){
				// Claim the new work as it's own
				reinterpret_cast<Instance*>(task.reference)->owner = this;
			}else{
				// Find new work in own queue
				task = this->queue.Find();

				// No task in unclaimed, or claimed work loads
				if (task.empty == true){
					break;
				}
			}
		#endif

		// Execute the task
		reinterpret_cast<Instance*>(task.reference)->Process(task.position);

		if (task.empty == true){
			break;
		}
	}


	// NOTE: MUST EXECUTE
	// Processing complete
	// Let the worker fall asleep
	this->changeState.lock();
	this->awake = false;
	this->changeState.unlock();
	return;
}





Thread::Pool::Pool(size_t threadCount): unclaimed(){
	// Initilize queue
	this->threads = threadCount;

	// Initilize workers
	for (size_t i=0; i<threadCount; i++){
		this->thread.push_back( new Thread::Worker(i, &this->unclaimed, this) );
	}
};

void Thread::Pool::Issue(EventLoop::Task task){
	this->unclaimed.Issue(task);

	// Find a sleeping worker and wait them in the hope they might consume the task
	for (size_t i=0; i<this->threads; i++){
		if (this->thread[i]->awake == false){
			this->thread[i]->Wake();

			// Succesfully waken one thread, then stop
			return;
		}
	}
};
bool Thread::Pool::Issue(size_t workerID, EventLoop::Task task){
	// Check the workerID is valid
	if (workerID < this->threads){
		this->thread[workerID]->Issue(task);
		return true;
	}

	return false;
};


bool Thread::Pool::HasActivity(){
	std::string msg;

	bool remainingWork = false;

	// Check if any unassigned work remains
	bool unassignedWork = this->unclaimed.HasTasks();
	if (unassignedWork == true){
		remainingWork = true;
	}

	// Check if any worker has work remaining and is asleep
	bool foundAwakeWorker = false;
	for (size_t i=0; i<this->threads; i++){
		if (this->thread[i]->awake == false){
			if (this->thread[i]->HasTasks() == true){
				msg = "Warn: Thread[" + std::to_string(i) + "] has fallen asleep while dedicated work is waiting.\n";
				std::cerr << msg;

				this->thread[i]->Wake();
				remainingWork = true;
			}
		}else{
			foundAwakeWorker = true;
		}
	};

	// If all workers are asleep, and unassigned work still exists
	if (unassignedWork == true && foundAwakeWorker == false){
		msg = "Warn: No workers are active while unassigned work still remains\n";
		std::cerr << msg;

		// Wake all workers
		for (size_t i=0; i<this->threads; i++){
			this->thread[i]->Wake();
		}
	}

	return remainingWork;
};


void Thread::Pool::WaitUntilDone(){
	size_t reps = 0;

	repeat:
	std::this_thread::sleep_for( std::chrono::milliseconds(100) );
	reps++;

	// Every 1000 repetitions
	if (reps > 1000){
		reps = 0;

		// Use pool's activity function since it'll wake any stalled workers
		if (this->HasActivity() == true){
			goto repeat;
		}
	}

	// Check if any workers are active every 100ms
	for (size_t i=0; i<this->threads; i++){
		if (this->thread[i]->awake == true){
			goto repeat;
		}
	}


	// If no one is active check the work pool before closing
	if (this->HasActivity()){
		goto repeat;
	};
};
