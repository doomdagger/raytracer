//
// Final Project for COMS 4998: C++ Library Design
// Author: He Li(hl2918), Haoxiang Xu(hx2185), Wangda Zhang(zwd)
// Credit to the book "C++ Concurrency in Action"
//

#ifndef RAYTRACER_THREAD_POOL_H
#define RAYTRACER_THREAD_POOL_H


#include <atomic>
#include <condition_variable>
#include <future>
#include <mutex>
#include <queue>
#include <thread>

namespace mmgl {

/**
 * This is a function wrapper that can handle move-only types.
 * Since std::packaged_task<> instances are not copyable, just movable, std::function<> cannot be used for the queue entries
 */
class function_wrapper {
        struct impl_base {
                virtual void call() = 0;
                virtual ~impl_base() {}
        };
        std::unique_ptr<impl_base> impl;
        template<typename F>
        struct impl_type : impl_base {
                F f;
                impl_type(F && f_) : f(std::move(f_)) {}
                void call() { f(); }
        };
public:
        template<typename F>
        function_wrapper(F&& f) : impl(new impl_type<F>(std::move(f))) {}
        void operator()() { impl->call(); }
        function_wrapper() = default;
        function_wrapper(function_wrapper&& other) : impl(std::move(other.impl)) {}
        function_wrapper& operator=(function_wrapper&& other) {
                impl = std::move(other.impl);
                return *this;
        }
        function_wrapper(const function_wrapper&) = delete;
        function_wrapper(function_wrapper&) = delete;
        function_wrapper& operator=(const function_wrapper&) = delete;
};

/**
 * Thread safe queue based on locks.
 * Implementation based on the book "C++ Concurrency in Action".
 */
template<typename T>
class thread_safe_queue {
        mutable std::mutex mut;
        std::queue<T> data_queue;
        std::condition_variable data_cond;
public:
        thread_safe_queue() {}
        void push(T new_value) {
                std::lock_guard<std::mutex> lk(mut);
                data_queue.push(std::move(new_value));
                data_cond.notify_one();
        }
        void wait_and_pop(T& value) {
                std::unique_lock<std::mutex> lk(mut);
                data_cond.wait(lk, [this]{
                        return !data_queue.empty();
                });
                value = std::move(data_queue.front());
                data_queue.pop();
        }
        std::shared_ptr<T> wait_and_pop() {
                std::unique_lock<std::mutex> lk(mut);
                data_cond.wait(lk, [this]{
                        return !data_queue.empty();
                });
                std::shared_ptr<T> res(std::make_shared<T>(std::move(data_queue.front())));
                data_queue.pop();
                return res;
        }
        bool try_pop(T& value) {
                std::lock_guard<std::mutex> lk(mut);
                if(data_queue.empty())
                        return false;
                value = std::move(data_queue.front());
                data_queue.pop();
                return true;
        }
        std::shared_ptr<T> try_pop() {
                std::lock_guard<std::mutex> lk(mut);
                if(data_queue.empty())
                        return std::shared_ptr<T>();
                std::shared_ptr<T> res(std::make_shared<T>(std::move(data_queue.front())));
                data_queue.pop();
                return res;
        }
        bool empty() const {
                std::lock_guard<std::mutex> lk(mut);
                return data_queue.empty();
        }
};

/**
 * Threads joiner that joins all threads in the pool in destructor.
 */
class join_threads {
        std::vector<std::thread>& threads;
public:
        explicit join_threads(std::vector<std::thread>& threads_) : threads(threads_) {}
        ~join_threads() {
                for(unsigned long i = 0; i < threads.size(); ++i) {
                        if(threads[i].joinable())
                                threads[i].join();
                }
        }
};

/**
 * Thread pool that provides better control over the number of threads used for rendering.
 * This implementation is based on the book "C++ Concurrency in Action".
 */
class thread_pool {
        std::atomic_bool done;
        thread_safe_queue<function_wrapper> work_queue;
        std::vector<std::thread> threads;
        join_threads joiner;

        void worker_thread() {
                while (!done) {
                        function_wrapper task;
                        if (work_queue.try_pop(task)) {
                                task();
                        } else {
                                std::this_thread::yield();
                        }
                }
        }

public:
        thread_pool(const unsigned thread_count = std::thread::hardware_concurrency()) : done(false), joiner(threads) {
                try {
                        for (unsigned i = 0; i < thread_count; ++i) {
                                threads.push_back(std::thread(&thread_pool::worker_thread, this));
                        }
                } catch (...) {
                        done = true;
                        throw;
                }
        }

        ~thread_pool() {
                done = true;
        }

        template<typename FunctionType>
        std::future<typename std::result_of<FunctionType()>::type> submit(FunctionType f) {
                typedef typename std::result_of<FunctionType()>::type result_type;
                std::packaged_task<result_type()> task(std::move(f));
                std::future<result_type> res(task.get_future());
                work_queue.push(std::move(task));
                return res;
        }
};

}

#endif // RAYTRACER_THREAD_POOL_H
