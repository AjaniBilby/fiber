#include "./eventloop.hpp"

namespace EventLoop{
	Schedule::Schedule(){
		this->activity.lock();
		this->progress = 0;

		// Create an empty queue
		// Label each slot as empty
		this->queue.resize( DEFAULT_QUEUE_LENGTH, Task{nullptr, 0, true} );

		this->activity.unlock();
	};

	void Schedule::Issue(Task task){
		task.empty = false; // Ensure that the slot is marked as in use

		// Prevent other treads from altering the queue while processing
		this->activity.lock();

		// Loop throug the queue to find an empty slot
		size_t size = this->queue.size();
		size_t i = 1 % size;
		while (i != 0){
			if (this->queue[i].empty == true){
				this->queue[i] = task;        // Transfer the task info

				// Allow other threads to alter the queue
				this->activity.unlock();
				return;
			}

			i = (i+1) % size;
		};

		// No empty slots were found
		// Expand the queue
		this->queue.push_back(task);

		// Allow other threads to alter the queue
		//  Now that editing is complete
		this->activity.unlock();
		return;
	};

	Task Schedule::Find(){
		Task out;

		// Prevent other treads from altering the queue while processing
		//  (also waits until other threads are finished before executing)
		this->activity.lock();

		// Loop throug the queue to find a task slot
		// Seach from previous seach endpoint to not disadvantage certain tasks
		size_t size = this->queue.size();
		size_t i = (this->progress+1) % size;
		while (i != this->progress){

			if (this->queue[i].empty == false){
				out = this->queue[i];         // Cache slot
				this->queue[i].empty = true;  // Mark the slot as empty
				this->progress = i;           // Mark start position for next search

				this->activity.unlock(); // Allow other threads to alter the queue
				out.empty = false;       // State that the result is valid
				return out;
			}

			i = (i+1) % size;
		};

		// Unable to find task
		// Allow other threads to access the queue
		this->activity.unlock();

		// Return an invalid result
		out.empty = true;
		return out;
	};

	bool Schedule::HasTasks(){
		// Prevent other threads from accessing the queue while searching
		this->activity.lock();

		// Find a non-empty slot
		size_t size = this->queue.size();
		for (size_t i=0; i<size; i++){
			// Is the slot empty?
			if (this->queue[i].empty == false){

				// The slot is not empty, so there are tasks available
				//  Stop itterating
				this->activity.unlock();
				return true;
			}
		}

		// Allow other threads to regain access to this queue
		this->activity.unlock();

		return false;
	};

	size_t Schedule::size(){
		return this->queue.size();
	};
};
