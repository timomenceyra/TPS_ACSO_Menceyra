/**
 * File: thread-pool.cc
 * --------------------
 * Presents the implementation of the ThreadPool class.
 */

#include "thread-pool.h"
#include <stdexcept>
using namespace std;

ThreadPool::ThreadPool(size_t numThreads) : wts(numThreads), done(false) {
    for (size_t i = 0; i < numThreads; i++) {
        wts[i].ts = thread([this, i]() {        // Create a worker thread
            worker(i);                          // Execute the worker function for each worker
        });
    }

    dt = thread([this]() {                      // Create the dispatcher thread
        dispatcher();
    });
}

void ThreadPool::schedule(const function<void(void)>& thunk) {
    if (!thp_active.load()) throw runtime_error("Cannot schedule on inactive ThreadPool"); // Check if the ThreadPool is active
    if (!thunk) throw invalid_argument("Thunk cannot be null"); // Check if the thunk is valid
    {
        lock_guard<mutex> lock(task_mtx);       // Lock the task mutex to safely access the task queue
        tasks.push(thunk);                      // Add the task to the queue
        {
            lock_guard<mutex> guard(wait_mtx);  // Lock the wait mutex
            activeTasks++;                      // Increment the count of active tasks
        }                  
    }

    task_ready.notify_all();                    // Notify the dispatcher that a new task is available
}

void ThreadPool::dispatcher() {
    while (true) {
        function<void(void)> task;

        {
            unique_lock<mutex> lock(task_mtx);                  // Lock the task mutex
            task_ready.wait(lock, [this]() {
                return !tasks.empty() || done;                  // Wait until there are tasks or the pool is done
            });
            if (done && tasks.empty()) return;                  // If done and no tasks, exit the loop
            task = tasks.front();                               // Move the task from the queue
            tasks.pop();                                        // Remove the task from the queue
        }

        bool taskAssigned = false;                              // Flag to check if the task was assigned
        while (!taskAssigned) {
            for (auto& worker : wts) {                          // Iterate through workers
                if (worker.available.load()) {                  // Check if the worker is available
                    worker.available.store(false);              // Mark the worker as not available
                    {
                        lock_guard<mutex> guard(worker.mtx);    // Lock the worker's mutex
                        worker.thunk = move(task);              // Assign the task to the worker
                    }
                    worker.ready.signal();                      // Signal the worker that a task is ready
                    taskAssigned = true;                        // Task has been assigned
                    break;                                      // Exit the loop once a task is assigned
                }
            }
            if (!taskAssigned) {
                this_thread::yield();                           // Yield if no workers are available
            }
        }
    }
}

void ThreadPool::worker(int id) {
    worker_t& worker = wts[id];                     // Get the worker by ID

    while (true) {
        worker.ready.wait();                        // Wait for a task to be ready
        if (done) break;                            // If done and no task, exit the loop

        function<void(void)> task;
        
        {
            lock_guard<mutex> guard(worker.mtx);    // Lock the worker's mutex
            task = move(worker.thunk);              // Move the task from the worker
            worker.thunk = nullptr;                 // Clear the worker's task
        }

        if (task) {                                 // If there is a task to execute
            task();                                 // Execute the task
        }

        worker.available.store(true);               // Mark the worker as available for new tasks

        {
            lock_guard<mutex> lock(wait_mtx);       // Lock the wait mutex
            activeTasks--;                          // Decrement the count of active tasks
            if (activeTasks == 0) {
                wait_cv.notify_all();               // Notify if all tasks are done
            }
        }
    }
}

void ThreadPool::wait() {
    unique_lock<mutex> lock(wait_mtx);  // Lock the wait mutex
    wait_cv.wait(lock, [this]() {
        return activeTasks == 0;        // Wait until all tasks are done
    });
}

ThreadPool::~ThreadPool() {
    wait();                                 // Wait for all tasks to complete

    thp_active.store(false);                // Mark the ThreadPool as inactive
    
    {
        lock_guard<mutex> lock(task_mtx);   // Lock the task mutex
        done = true;                        // Set the done flag to true
    }

    task_ready.notify_all();                // Notify the dispatcher that the pool is done

    if (dt.joinable()) {
        dt.join();                          // Join the dispatcher thread
    }

    for (auto& worker : wts) {
        worker.ready.signal();              // Signal each worker to wake up and exit
        if (worker.ts.joinable()) {
            worker.ts.join();               // Join each worker thread
        }
    }
}
