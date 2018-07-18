#include "./thread.hpp"




namespace Thread{
	Schedule::Schedule(){
		this->data.resize(Thread_Default_Queue_Size);
		this->empty.resize(Thread_Default_Queue_Size, true);

		this->lastFound = 0;
		this->jobs = 0;
	}

	void Schedule::Dispatch(Job job){
		this->active.lock();
		this->jobs += 1;

		// Find an empty slot
		// Place the entry within said slot
		int length = this->data.capacity();
		for (int i=0; i<length; i++){
			if ( this->empty[i] == true ){
				this->data[i] = job;
				this->empty[i] = false;

				this->active.unlock();
				return;
			}
		}

		// Expand the schedule size to fit
		// Place the entry at the first new index
		data.resize( length + Thread_Queue_Increment );
		empty.resize( data.capacity(), true );
		this->data[length] = job;
		this->empty[length] = false;

		this->active.unlock();
		return;
	}

	JobResult Schedule::Search(){
		JobResult res;

		// Ensure no other thread is manipulating the Schedule
		this->active.lock();

		// Search for an available job,
		// Start for the previous searches end point,
		// This will remove the advantage of as task being at the
		//   front of the queue
		unsigned int length = this->data.capacity();
		unsigned int i=this->lastFound;
		while (true){
			if (this->empty[i] == false){

				// Parse the information to the result
				res.found = true;
				res.result = this->data[i];

				// Remove this job from the queue
				this->lastFound = i;
				this->empty[i] = true;
				this->jobs -= 1;

				// Return and wrap up operation
				this->active.unlock();

				return res;
			}

			// Itterate looped scan
			i++;
			if (i >= length){
				i = 0;
			}

			// A full search of the Schedule has been done
			// No results found, break search
			if (i == this->lastFound){
				break;
			}
		}

		this->active.unlock();
		res.found = false;
		return res;
	};


	void Schedule::Recall(void *ptr){
		this->active.lock();

		unsigned long length = this->data.capacity();
		for (unsigned long i=0; i<length; i++){
			if (this->empty[i] == false){
				if (this->data[i].ptr == ptr){
					this->empty[i] = true;
				}
			}
		}

		this->active.unlock();
	}


	unsigned long Schedule::JobCount(){
		return this->jobs;
	};

	unsigned long Schedule::Capacity(){
		return this->data.capacity();
	};
}






namespace Thread{
	Worker::Worker(unsigned int id, Schedule *unassignedHeap){
		this->id = id;
		this->anonymous = unassignedHeap;
		this->thread = nullptr;
		this->active = false;
	};

	bool Worker::IsActive(){
		this->sensitive.lock();

		if (this->active){
			this->sensitive.unlock();
			return true;
		}

		this->sensitive.unlock();
		return false;
	}

	bool Worker::Wake(){
		this->sensitive.lock();

		if (this->active == false){

			if (this->thread != nullptr){
				std::string msg;
				msg = "Thread["+std::to_string(this->id)+"] instance already exists, deleting...\n";
				std::cout << msg;
				delete this->thread;
			}

			this->active = true;
			this->thread = new std::thread(&Worker::Process, this);
			this->thread->detach();

			this->sensitive.unlock();
			return true;
		}

		this->sensitive.unlock();
		return false;
	};

	void Worker::Sleep(){
		this->sensitive.lock();

		this->active = false;
		if (this->thread != nullptr){
			delete this->thread;
			this->thread = nullptr;
		}

		this->sensitive.unlock();

		std::string msg;
		msg = "Thread["+std::to_string(this->id)+"]: Sleep\n";
		std::cout << msg;
	}

	void Worker::Assign(Job task){
		this->work.Dispatch(task);

		// Ensure that the thread is awake to process the task
		this->Wake();
	};

	void Worker::Recall(void *ptr){
		this->work.Recall(ptr);
	};

	unsigned int Worker::JobCount(){
		return this->work.JobCount();
	};
};






namespace Thread{
	Pool::Pool(unsigned int threads){
		// If unspecified, use system info
		if (threads == 0){
			threads = std::thread::hardware_concurrency();
		}
		// Min workers = 2
		if (threads < 2){
			threads = 2;
		}

		this->workers = threads;
		this->worker.reserve(threads);

		// Assign Worker's default values
		for (int i=0; i<threads; i++){
			this->worker[i] = new Worker(i, &this->anonymous);
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
				if (this->worker[i]->Wake() == true){
					break;
				}
			}
		}else{
			if (workerID > workers){
				std::cerr << "Error: Attempting to assign task to invalid worker" << std::endl;
				return;
			}

			this->worker[workerID]->Assign(task);
		}
	};

	bool Pool::Active(){
		std::string msg;

		for (int i=0; i<workers; i++){
			if (this->worker[i]->IsActive() == true){
				msg = "Active thread["+std::to_string(i)+"]\n";
				std::cout << msg;
				return true;
			}
		}

		return false;
	};

	void Pool::WakeAll(){
		for (int i=0; i<this->workers; i++){
			this->worker[i]->Wake();
		}
	};

	unsigned long Pool::JobCount(){
		unsigned int tally = this->anonymous.JobCount();

		for (int i=0; i<this->workers; i++){
			tally += this->worker[i]->JobCount();
		}

		return tally;
	};

	void Pool::Wedge(){
		bool active = true;
		unsigned long remain;

		std::string msg;

		while (active){
			active = this->Active();

			if (!active){
				std::cout << "No threads active" << std::endl;
				remain = this->JobCount();
				if (remain > 0){
					msg = "Warn: All threads were asleep while " + std::to_string(remain) + " jobs remained\n";
					std::cerr << msg;

					this->WakeAll();
					active = true;

					continue;
				}

				std::cout << "  closing..." << std::endl;
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}

		std::cout << "Wedge end" << std::endl;
	};
};



#include "instance.hpp"

namespace Thread{
	void Worker::Process(){
		JobResult res;
		Instance *target;

		std::string msg;

		// Repeat until no task is found
		while (true){
			// Reset
			res.found = false;

			// Find a job from either own work load or anonymous
			//   Priorities own work over anonoymous
			res = this->work.Search();
			if (res.found == false){
				res = this->anonymous->Search();

				// Claim the anonymous instance
				if (res.found){
					target = reinterpret_cast<Instance*>(res.result.ptr);
					target->sensitive.lock();
					target->workerID = this->id;
					target->assigned = true;
					target->sensitive.unlock();
				}
			}else{
				target = reinterpret_cast<Instance *>(res.result.ptr);
			}

			if (res.found == true){
				// If the task is to destory the instance
				//  Do that instead of execution
				if (res.result.conclude){
					target->Destory();
					continue;
				}

				msg = "Thread["+std::to_string(this->id)+"]: Executing\n";
				std::cout << msg;
				target->Execute(res.result.cursor, res.result.data, res.result.size);
				msg = "Thread["+std::to_string(this->id)+"]: Executed\n";
				std::cout << msg;
				// Find next job
				continue;
			}else{
				break;
			}
		}

		this->Sleep();
	};


	void Pool::Recall(void *ptr){
		Instance *target = reinterpret_cast<Instance *>(ptr);
		target->sensitive.lock();

		this->anonymous.Recall(ptr);

		if (target->assigned){
			this->worker[ target->workerID ]->Recall(ptr);
		}

		target->sensitive.unlock();
	}
}
