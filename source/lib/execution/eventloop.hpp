#pragma once

#include "./../../flags.hpp"

#include "./../typebase.hpp"

#include <vector>
#include <mutex>


namespace EventLoop{
	struct Task{
		void* reference;
		size_t position;

		bool empty;
	};

	class Schedule{
		public:
			Schedule();

			void Issue(Task task);
			Task Find(); // Return a value rather than a reference incase the slot is instantly refilled after finding it
			bool HasTasks();
			size_t size();
		private:
			size_t progress;

			std::vector<Task> queue;
			std::mutex activity;
	};
};


#include "./eventloop.cpp"
