#pragma once

#include <thread>
#include <vector>
#include <mutex>



#define Thread_Default_Queue_Size 10
#define Thread_Queue_Increment 10



namespace Thread{
	struct Job{
    void *ptr            = nullptr;
    unsigned long cursor = 0;
  };

	struct JobResult{
		bool found;
		Job result;
	};


	class Schedule{
		private:
			std::vector<Job> data;
			std::vector<bool> empty;

			std::mutex active;
			unsigned long lastFound = 0;
			unsigned long jobs = 0;

		public:
			Schedule();

			/*
				Adds the job to the queue
				@param job - task details
			*/
			void Dispatch(Job job);
			/*
				Search for a job
				Only results jobs that are able to be processed by the requestor
				@param workerID - Who is attempting to process the task
			*/
			JobResult Search(int workerID);
			unsigned long JobCount();
			unsigned long Capacity();
	};


	class Worker{
		private:
			bool awake = false;
			void Process();

			Schedule work;
			Schedule *anonymous;
			unsigned long id = 0;

			std::thread *thread;

		public:
			Worker(unsigned int id, Schedule *unassignedHeap);
			/*
				Will ensure that the worker's thread has not collapsed
			*/
			bool Wake();
			/*
				Gives the worker a job to put in it's queue
			*/
			void Assign(Job task);
			/*
				Returns if the worker is active or not
			*/
			bool Active(){
				return this->awake;
			};

			/*
				Returns the number of assigned jobs
			*/
			unsigned int JobCount();
	};


	class Pool{
		private:
			void WakeAll();

			std::vector<Worker*> worker;
			unsigned long workers;

			Schedule anonymous;

		public:
			/*
				@param threads - The number of workers spawned by the pool
			*/
			Pool(unsigned int threads);
			/*
				Put the task into the correct schedule for processing
				@param task - details the work to be done
				@param targeted - whether or not the job is for a specific thread
				@param workerID - which worker if any the task is targeted at
			*/
			void Dispatch(Job task, bool targeted, unsigned int workerID);
			/*
				Stops program exiting before workers have finished all of their work
			*/
			void Wedge();
			/*
				@returns - whether or not any workers are active
			*/
			bool Active();

			/*
				@returns - the number of Jobs to be processed
			*/
			unsigned long JobCount();
			unsigned long ThreadCount(){
				return this->workers;
			};
	};
};



#include "thread.cpp"
