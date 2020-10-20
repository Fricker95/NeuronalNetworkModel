//
//  ThreadPool.hpp
//  NeuronalNetwork
//
//  Created by Nicolas Fricker on 12/18/19.
//  Copyright © 2019 Nicolas Fricker. All rights reserved.
//

#ifndef ThreadPool_
#define ThreadPool_

#pragma GCC visibility push(hidden)

#include <iostream>
#include <algorithm>
#include <atomic>
#include <string>
#include <vector>

#include <pthread.h>

template <class thread, class queue, class result>
class ThreadPool {
	// total count of active threads
	size_t total_ = 0;
	// atomic variable storing the progress
	std::atomic<int> progress_{0};
	// array of thread objects
	std::vector<thread> threads_;
	// array of queue objects
	std::vector<queue> q_;
	// array of result objects
	std::vector<result> results_;

 public:
	explicit ThreadPool(int n_threads = 1, int n_items = 1) {
		threads_.reserve(n_threads);
		q_.reserve(n_items);
		// thread implicit initialization
		for (size_t i = 0; i < n_threads; ++i)
			threads_.emplace_back(&q_, &results_, &progress_);
	}
	
	~ThreadPool() {
		if (!stopped() && started()) {
			stop();
			join();
		}
		results_.clear();
		clear();
	}
	
	template <typename... Args>
	inline void set_task(Args... args) noexcept {
		/*
			Add task to queue
		*/
		q_.emplace_back(args...);
	}
	
	const void start(bool sort = false) noexcept {
		/*
			sets the total variable based on how many tasks are in the queue
			launches threads
		*/
		progress_ = 0;

		if (threads_.size() < q_.size()) {
			total_ = threads_.size();
		} else {
			total_ = q_.size();
		}
		for (size_t i = 0; i < total_; ++i)
			threads_[i].start();
	}
	
	const void stop() noexcept {
		/*
			asks thread execution to stop
		*/
		for (size_t i = 0; i < total_; ++i) {
			if (!threads_[i].stopped() && threads_[i].started())
				threads_[i].stop();
		}
	}
	
	const void cancel() noexcept {
		/*
			forces thread exectution to stop
			does not delete allocated objects
		*/
		for (size_t i = 0; i < total_; ++i)
			threads_[i].cancel();
	}
	
	std::vector<result>* join() noexcept {
		/*
			waits for the threads to finish execution and returns the results
		*/
		for (size_t i = 0; i < total_; ++i) {
			(void)threads_[i].join();
		}
		return &results_;
	}

	const bool kill() noexcept {
		/*
			kill function
		*/
		bool a = false;
		for (int i = 0; i < total_; ++i) {
			a = threads_[i].kill();
		}
		return a;
	}
	
	inline const size_t size() noexcept {
		/*
			returns queue size
		*/
		return q_.size();
	}
	
	inline const bool empty() noexcept {
		/*
			checks if queue is empty
		*/
		return q_.empty();
	}
	
	inline const void clear() noexcept {
		/*
			empties the queue
		*/
		q_.clear();
	}
	
	inline const int progress() noexcept {
		/*
			returns progresss
		*/
		return progress_;
	}
	
	inline std::vector<result>* results() noexcept {
		/*
			returns results
		*/
		return &results_;
	}
	
	const bool started() noexcept {
		/*
			checks active threads have started
		*/
		bool did_start = false;
		for (size_t i = 0; i < total_; ++i) {
			if (threads_[i].started())
				did_start = true;
		}
		return did_start;
	}
	
	const bool stopped() noexcept {
		/*
			checks threads have stopped
		*/
		bool is_finished = true;
		for (size_t i = 0; i < total_; ++i) {
			if (!threads_[i].stopped())
				is_finished = false;
		}
		return is_finished;
	}

	class Thread_ {
		// thread id
		pthread_t id_;
		// thread attributed: detached
		pthread_attr_t attribute_;
		
		bool started_ = false;
		bool stopped_ = false;
		bool detached_ = false;
		void* result_ = nullptr;

	 public:
		Thread_() {}
		
		virtual ~Thread_() {}
		
		pthread_t& id() {
			/*
			 returns thread id
			*/
			return id_;
		}
		
		const void start() {
			/*
				starts thread execution
			*/
			stopped_ = false;
			started_ = true;
			int status;
			status = pthread_attr_init(&attribute_);
			status = pthread_attr_setscope(&attribute_, PTHREAD_SCOPE_SYSTEM);
			
			if (!detached_) {
				status = pthread_create(&id_, NULL, Thread_::launch, reinterpret_cast<void*>(this));
				if (status)
					printf("%p: start status error %i\n", id_, status);
			} else {
				status = pthread_attr_setdetachstate(&attribute_, PTHREAD_CREATE_DETACHED);
				status = pthread_create(&id_, NULL, Thread_::launch, reinterpret_cast<void*>(this));
				if (status)
					printf("%p: start status error %i\n", id_, status);
			}
		}
		
		inline const void stop() noexcept {
			/*
				set stop flag to true
			*/
			stopped_ = true;
		}
		
		inline const void detach() noexcept {
			/*
				set detach flag to true
			*/
			detached_ = true;
		}
		
		inline void* join() noexcept {
			/*
				join thread and return result
			*/
			int status = pthread_join(id_, &result_);
			if (status)
				printf("%p: join status error %i\n", id_, status);
			return result_;
		}
		
		inline const void cancel() noexcept {
			/*
				cancels thread execution
			*/
			// Does not deinit allocated objects in thread, causes memory leak
			pthread_cancel(id_);
		}
		
		inline const bool started() noexcept {
			/*
				returns true if the thread has started
			*/
			return started_;
		}
		
		inline const bool stopped() noexcept {
			/*
				returns true if the thread has stopped
			*/
			return stopped_;
		}
		
		inline const bool detached() noexcept {
			/*
				returns true fi the thread is detached
			*/
			return detached_;
		}

		inline const bool kill() noexcept {
			/*
				kill function
			*/
			return pthread_kill(id_, 0) != 0;
		}

	 private:
		// virtual function run
		virtual void* run() noexcept = 0;
		
		inline const void finished() noexcept {
			/*
				resets thread state flags
			*/
			stopped_ = true;
			started_ = false;
		}
			
		// move constructor
		Thread_(Thread_&& other);
			
		// copy assignemnt
		Thread_& operator=(const Thread_& other) = default;
		
		inline static void* launch(void* pVoid) noexcept {
			/*
				main thread execution of the run function
			*/
			Thread_* pthread = reinterpret_cast<Thread_*>(pVoid);
			if (pthread) {
				pthread->result_ = pthread->run();
				pthread->finished();
				pthread_exit(pthread->result_);
			}
			pthread_exit(nullptr);
		}
	};
};

#pragma GCC visibility pop
#endif /* ThreadPool_ */
