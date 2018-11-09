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
		task.empty = false; // Ensure that the slot is marked as in use
		this->queue.push_back(task);

		return;
	};

	Task Schedule::Find(){
		// Prevent other threads from altering while this task is active
		std::lock_guard<std::mutex> lck( this->activity );

		Task out;

		if (this->queue.empty() == true){
			out.empty = true;
			return out;
		}

		out = this->queue.front();
		this->queue.pop_front();


		return out;
	};

	bool Schedule::HasTasks(){
		// Prevent other threads from altering while this task is active
		std::lock_guard<std::mutex> lck( this->activity );

		return this->queue.empty() == false;
	};
};
