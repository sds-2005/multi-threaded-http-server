#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <iostream>

using namespace std;

class ThreadPool {
public:
    // Constructor- Launches worker threads
    ThreadPool(size_t threads) : stop(false) {
        for(size_t i = 0; i < threads; ++i)
            workers.emplace_back(
                [this] {
                    for(;;) {
                        function<void()> task;

                        {
                            // LOCK THE QUEUE
                            unique_lock<mutex> lock(this->queue_mutex);
                            
                            // WAIT UNTIL JOB AVAILABLE OR STOPPED
                            this->condition.wait(lock,
                                [this]{ return this->stop || !this->tasks.empty(); });
                            
                            // EXIT CONDITION
                            if(this->stop && this->tasks.empty())
                                return;
                            
                            // GET THE JOB
                            task = move(this->tasks.front());
                            this->tasks.pop();
                        }

                        // EXECUTE THE JOB (UNLOCKED)
                        task();
                    }
                }
            );
    }

    // Add a new job to the pool
    template<class F, class... Args>
    void enqueue(F&& f, Args&&... args) {
        {
            unique_lock<mutex> lock(queue_mutex);

            // Don't allow enqueueing after stopping
            if(stop)
                throw runtime_error("enqueue on stopped ThreadPool");

            // Wrap the function and arguments into a task
            tasks.emplace(bind(forward<F>(f), forward<Args>(args)...));
        }
        condition.notify_one();
    }

    // Destructor- Joins all threads
    ~ThreadPool() {
        {
            unique_lock<mutex> lock(queue_mutex);
            stop = true;
        }
        condition.notify_all();
        for(thread &worker: workers)
            worker.join();
    }

private:
    vector<thread> workers;
    queue<function<void()>> tasks;
    
    mutex queue_mutex;
    condition_variable condition;
    bool stop;
};

#endif