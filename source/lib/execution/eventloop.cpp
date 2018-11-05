#include "./eventloop.hpp"

namespace EventLoop{
	Schedule::Schedule(){
		this->activity.lock();

		// Create an empty queue
		// Label each slot as empty
		this->queue.resize( DEFAULT_QUEUE_LENGTH, Task{nullptr, 0, true} );

		this->activity.unlock();
	};

	void Schedule::Issue(Task task){
		this->activity.lock(); // Prevent other treads from altering the queue while processing

		// Loop throug the queue to find an empty slot
		size_t size = this->queue.size();
		size_t i = 1 % size;
		while (i != 0){
			if (this->queue[i].empty == true){
				this->queue[i] = task;
				this->queue[i].empty = false;

				this->activity.unlock(); // Allow other threads to alter the queue
				return;
			}

			i = (i+1) % size;
		};

		// No empty slots were found
		// Expand the queue
		this->queue.push_back(task);
		this->queue[size].empty = false;

		this->activity.unlock();  // Allow other threads to alter the queue
		return;
	};

	Task Schedule::Find(){
		Task out;

		this->activity.lock(); // Prevent other treads from altering the queue while processing

		// Loop throug the queue to find an task slot
		size_t size = this->queue.size();
		size_t i = this->progress % size;
		while (i != this->progress){
			if (this->queue[i].empty == false){
				out = this->queue[i];         // Transfer the slot for caching
				this->queue[i].empty = false; // Mark the slot as empty

				this->activity.unlock(); // Allow other threads to alter the queue
				return out;
			}

			i = (i+1) % size;
		};

		// Unable to find task
		out.empty = true;
		return out;
	};

	bool Schedule::HasTasks(){
		bool hasTasks = false;

		this->activity.lock();

		// Find a non-empty slot
		size_t size = this->queue.size();
		for (size_t i=0; i<size; i++){
			if (this->queue[i].empty == false){
				hasTasks = true;
				break;
			}
		}

		this->activity.unlock();

		return hasTasks;
	};

	size_t Schedule::size(){
		return this->queue.size();
	};
};
