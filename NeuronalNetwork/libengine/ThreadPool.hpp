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
	size_t total_ = 0;
	std::atomic<int> progress_{0};
	std::vector<thread> threads_;
	std::vector<queue> q_;
	std::vector<result> results_;

 public:
	explicit ThreadPool(int n_threads = 1, int n_items = 1) {
		threads_.reserve(n_threads);
		q_.reserve(n_items);
		for (size_t i = 0; i < n_threads; ++i)
			threads_.emplace_back(&q_, &results_, &progress_);
	}
	
	~ThreadPool() {
		results_.clear();
		clear();
	}
	
	template <typename... Args>
	inline void set_task(Args... args) noexcept {
		q_.emplace_back(args...);
	}
	
	void start(bool sort = false) noexcept {
		progress_ = 0;

		if (threads_.size() < q_.size()) {
			total_ = threads_.size();
		} else {
			total_ = q_.size();
		}
		for (size_t i = 0; i < total_; ++i)
			threads_[i].start();
	}
	
	void stop() noexcept {
		for (size_t i = 0; i < total_; ++i) {
			if (!threads_[i].stopped() && threads_[i].started())
				threads_[i].stop();
		}
	}
	
	void cancel() noexcept {
		for (size_t i = 0; i < total_; ++i)
			threads_[i].cancel();
	}
	
	std::vector<result>* join() noexcept {
		for (size_t i = 0; i < total_; ++i) {
			(void)threads_[i].join();
		}
		return &results_;
	}

	bool kill() noexcept {
		bool a = false;
		for (int i = 0; i < total_; ++i) {
			a = threads_[i].kill();
		}
		return a;
	}
	
	inline size_t size() noexcept {
		return q_.size();
	}
	
	inline bool empty() noexcept {
		return q_.empty();
	}
	
	inline void clear() noexcept {
		q_.clear();
	}
	
	inline int progress() noexcept {
		return progress_;
	}
	
	inline std::vector<result>* results() noexcept {
		return &results_;
	}
	
	bool started() noexcept {
		bool did_start = false;
		for (size_t i = 0; i < total_; ++i) {
			if (threads_[i].started())
				did_start = true;
		}
		return did_start;
	}
	
	bool stopped() noexcept {
		bool is_finished = true;
		for (size_t i = 0; i < total_; ++i) {
			if (!threads_[i].stopped())
				is_finished = false;
		}
		return is_finished;
	}

	class Thread_ {
		pthread_t id_;
		pthread_attr_t attribute_;
		
		bool started_ = false;
		bool stopped_ = false;
		bool detached_ = false;
		void* result_ = nullptr;

	 public:
		Thread_() {}
		
		virtual ~Thread_() {}
		
		pthread_t& id() {
			return id_;
		}
		
		void start() {
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
		
		inline void stop() noexcept {
			stopped_ = true;
		}
		
		inline void detach() noexcept {
			detached_ = true;
		}
		
		inline void* join() noexcept {
			int status = pthread_join(id_, &result_);
			if (status)
				printf("%p: join status error %i\n", id_, status);
			return result_;
		}
		
		inline void cancel() noexcept {
			// Does not deinit allocated objects in thread, causes memory leak
			pthread_cancel(id_);
		}
		
		inline bool started() noexcept {
			return started_;
		}
		
		inline bool stopped() noexcept {
			return stopped_;
		}
		
		inline bool detached() noexcept {
			return detached_;
		}

		inline bool kill() noexcept {
			return pthread_kill(id_, 0) != 0;
		}

	 private:
		virtual void* run() noexcept = 0;
		
		inline void finished() noexcept {
			stopped_ = true;
			started_ = false;
		}
			
		Thread_(Thread_&& other);
		
		Thread_& operator=(const Thread_& other) = default;
		
		static void* launch(void* pVoid) noexcept {
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
