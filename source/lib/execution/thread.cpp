#include "./thread.hpp"




Thread::Worker::Worker(size_t wID, EventLoop::Schedule* wPool, void* tPool){
	this->workerID    = wID;
	this->workPool    = wPool; // Receive unassigned work pool
	this->threadPool  = tPool;

	this->shouldClose = false;
	this->active      = false;
	this->sysThread   = new std::thread(&Thread::Worker::Process, this);
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
	if (this->active == false){
		this->active = true;
		std::unique_lock<std::mutex> lck(this->mtx);
		this->ping.notify_all();

		// A change occured
		return true;
	}

	// The worker was already awake
	return false;
};
void Thread::Worker::Close(){
	this->shouldClose = true;

	// If the thread is suspended wake it so it can decay
	this->Wake();

	// Merge the thread to this one
	this->sysThread->join();
};

bool Thread::Worker::HasTasks(){
	return this->queue.HasTasks();
};

EventLoop::SearchResult Thread::Worker::FindTask(){
	EventLoop::SearchResult res;
	res.found = false;

	// See flags.hpp for pre-processor variable definition
	#if (THREAD_OWN_TASK_PRIORITY)
		// Search for work in designated heap
		res = this->queue.Find();
		if (res.found == true){
			return res;
		}

		// No result
		// Each in unallocated work pool
		res = this->workPool->Find();
		if (res.found == true){
			// Claim the new instance as it's own
			reinterpret_cast<Instance*>(res.data.reference)->owner = this;
			return res;
		}
	#else
		// No result
		// Each in unallocated work pool
		res = this->workPool->Find();
		if (res.found == true){
			// Claim the new instance as it's own
			reinterpret_cast<Instance*>(res.data.reference)->owner = this;
			return res;
		}

		// Search for work in designated heap
		res = this->queue.Find();
		if (res.found == true){
			return res;
		}
	#endif

	return res;
};

void Thread::Worker::Process(){
	this->active = true;
	std::unique_lock<std::mutex> lck(this->mtx);
	std::string str;

	#if DEBUG
	str = "Worker " + std::to_string(this->workerID) + " generated\n";
	std::cout << str;
	#endif

	EventLoop::SearchResult result;
	while (true){
		result = this->FindTask();

		if (result.found == true){
			// Execute the task
			#if DEBUG
			str = "Worker " + std::to_string(this->workerID) + " processing..\n";
			std::cout << str;
			#endif
			reinterpret_cast<Instance*>(result.data.reference)->Process(result.data.position);
		}else{
			// Suspend the thread until notified of new tasks
			this->active = false;
			#if DEBUG
			str = "Worker " + std::to_string(this->workerID) + " sleeping..\n";
			std::cout << str;
			#endif
			this->ping.wait(lck);
			#if DEBUG
			str = "Worker " + std::to_string(this->workerID) + " woken..\n";
			std::cout << str;
			#endif
			this->active = true;

			if (this->shouldClose){
				break;
			}else{
				continue;
			}
		}
	}

	#if DEBUG
	str = "Worker " + std::to_string(this->workerID) + " destorying\n";
	#endif
	std::cout << str;

	return;
};





Thread::Pool::Pool(size_t threadCount): unclaimed(){
	// Initilize queue
	this->threads = threadCount;

	// Initilize workers
	for (size_t i=0; i<threadCount; i++){
		this->thread.push_back( new Thread::Worker(i, &this->unclaimed, this) );
	}
};

void Thread::Pool::Issue(EventLoop::Task task){
	// Post task to unclaimed queue
	this->unclaimed.Issue(task);

	// Find a suspended work and issue the task to them
	for (size_t i=0; i<this->threads; i++){
		if (this->thread[i]->Wake() == true){
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
		if (this->thread[i]->active == false){
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
		if (this->thread[i]->active == true){
			goto repeat;
		}
	}


	// If no workers are active check the work pool before closing
	//  (incase this isn't a 1000th rep)
	if (this->HasActivity()){
		goto repeat;
	};
};

void Thread::Pool::Close(){
	for (size_t i=0; i<this->threads; i++){
		this->thread[i]->Close();
	}
};
