#include "./thread.hpp"




Thread::Worker::Worker(size_t wID, EventLoop::Schedule* wPool, void* tPool){
	this->workerID = wID;
	this->workPool = wPool;               // Receive unassigned work pool
	this->threadPool = tPool;

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

bool Thread::Worker::Wake(){

	// If the thread isn't currently active
	if (this->IsActive() == false){
		// Revive the thread
		std::thread (&Thread::Worker::Process, this).detach();

		return true;
	}

	return false;
};

bool Thread::Worker::HasTasks(){
	return this->queue.HasTasks();
};
bool Thread::Worker::IsActive(){
	// Is activity already locked?
	if (this->activity.try_lock() == true){
		// Succeded in locking, therefor it is not active

		this->activity.unlock(); // Alow the thread to lock it's activity in the future
		return false;
	}

	// The thread is currently active
	return true;
};

void Thread::Worker::Process(){
	this->activity.lock();

	std::string str = "Thread " + std::to_string(this->workerID) + " waking...\n";
	std::cout << str;

	EventLoop::Task task;
	while (true){

		// See flags.hpp for pre-processor variable definition
		#if (THREAD_OWN_TASK_PRIORITY)
			// Search for work in designated heap
			task = this->queue.Find();
			if (task.empty == true){
				// Search for work in unclaimed work pool
				task = this->workPool->Find();

				// Unable to find a task in own pool or unclaimed
				if (task.empty == true){
					break;
				}

				// Claim the new instance as it's own
				reinterpret_cast<Instance*>(task.reference)->owner = this;
			}
		#else
			task = this->workPool->Find();
			if (task.empty == true){
				// Find new work in own queue
				task = this->queue.Find();

				// No work found in own pool or unclaimed
				if (task.empty == true){
					break;
				}
			}else{
				// Claim the new instance as it's own
				reinterpret_cast<Instance*>(task.reference)->owner = this;
			}
		#endif

		// Execute the task
		reinterpret_cast<Instance*>(task.reference)->Process(task.position);
		std::cout << "  Processing...\n";
		std::this_thread::sleep_for( std::chrono::milliseconds(100) );
	}


	// NOTE: MUST EXECUTE
	// Processing complete
	// Let the worker fall asleep
	this->activity.unlock();

	str = "Thread " + std::to_string(this->workerID) + " sleeping...\n";
	std::cout << str;

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
	std::this_thread::sleep_for( std::chrono::milliseconds(100) );

	// Find a sleeping worker and wait them in the hope they might consume the task
	for (size_t i=0; i<this->threads; i++){
		if (this->thread[i]->Wake() == true){
			// Succesfully woken one thread, then stop
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
		if (this->thread[i]->IsActive() == false){
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
		if (this->thread[i]->IsActive() == true){
			goto repeat;
		}
	}

	std::clog << "Warn: No workers active\n";


	// If no workers are active check the work pool before closing
	//  (incase this isn't a 1000th rep)
	if (this->HasActivity()){
		goto repeat;
	};
};
