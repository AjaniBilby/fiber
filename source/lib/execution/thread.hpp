#pragma once

#include "./eventloop.hpp"

#include <condition_variable>
#include <chrono>
#include <string>
#include <vector>
#include <mutex>


namespace Thread{
	class Worker{
		public:
			// This worker's unqiue ID (relative to threadpool)
			size_t workerID;

			bool shouldClose;
			bool active;

			//     This work's unique ID (unqiue to pool)
			//                      Address of unassigned work queue
			//                                                       Address of thread pool
			Worker(size_t wID, EventLoop::Schedule* unassigned, void* threadPool);

			// Put the task in own work pool
			void Issue(EventLoop::Task task);
			// Forward the task to the unassigned work pool
			void IssueToWorkPool(EventLoop::Task task);

			// Wake up the worker, ensuring the worker won't create a double ganger
			// Return: did the thread just wake?
			bool Wake();
			// Force the thread to join with the caller's thread
			void Close();

			bool HasTasks();
		private:
			// Own unique work queue
			EventLoop::Schedule queue;
			// Generic unassigned work pool
			EventLoop::Schedule* workPool;
			// Pointer to thread pool
			void* threadPool;

			std::thread *sysThread;

			// Work through queue
			void Process();

			EventLoop::SearchResult FindTask();

			// Used for notifying the threads of events
			std::condition_variable ping;
			std::mutex mtx;
	};


	class Pool{
		public:
			Pool(size_t threads);

			void Issue(EventLoop::Task task);
			bool Issue(size_t workerID, EventLoop::Task task);

			bool HasActivity();
			void WaitUntilDone();
			void Close();
		private:
			std::vector<Worker*> thread;
			size_t threads;

			EventLoop::Schedule unclaimed;
	};
};


#include "./instance.hpp"


#include "./thread.cpp"
