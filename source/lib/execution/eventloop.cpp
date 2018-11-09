#include "./eventloop.hpp"

namespace EventLoop{
	Schedule::Schedule(){
		// Prevent other threads from altering while this task is active
		std::lock_guard<std::mutex> lck( this->activity );
	};

	void Schedule::Issue(Task task){
		// Prevent other threads from altering while this task is active
		std::lock_guard<std::mutex> lck( this->activity );

		// Add the task to the queue
		this->queue.push_back(task);

		return;
	};

	SearchResult Schedule::Find(){
		// Prevent other threads from altering while this task is active
		std::lock_guard<std::mutex> lck( this->activity );

		SearchResult out;

		// The queue is empty
		// Therefor there will be no valid results
		if (this->queue.empty() == true){
			out.found = false;
			return out;
		}

		// Parse result
		out.found = true;               // State found result
		out.data = this->queue.front(); // Cache the result
		this->queue.pop_front();        // Remove the task from the queue

		return out;
	};

	bool Schedule::HasTasks(){
		// Prevent other threads from altering while this task is active
		std::lock_guard<std::mutex> lck( this->activity );

		return this->queue.empty() == false;
	};
};
