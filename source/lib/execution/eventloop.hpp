#pragma once

#include "./../../flags.hpp"

#include "./../typebase.hpp"

#include <deque>
#include <mutex>


namespace EventLoop{
	struct Task{
		void* reference;
		size_t position;
	};

	struct SearchResult{
		Task data;
		bool found;
	};

	class Schedule{
		public:
			Schedule();

			void Issue(Task task);
			SearchResult Find(); // Return a value rather than a reference incase the slot is instantly refilled after finding it
			bool HasTasks();
		private:
			std::deque<Task> queue;
			std::mutex activity;
	};
};


#include "./eventloop.cpp"
