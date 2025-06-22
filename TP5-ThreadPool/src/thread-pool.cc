/**
 * File: thread-pool.cc
 * --------------------
 * Presents the implementation of the ThreadPool class.
 */

#include "thread-pool.h"
using namespace std;

ThreadPool::ThreadPool(size_t numThreads) : wts(numThreads), done(false) {
    for (size_t i = 0; i < numThreads; i++) {
        wts[i].ts = thread([this, i]() {            // Create a worker thread
            worker(i);                              // Execute the worker function for each worker
        });
    }

    dt = thread([this]() {                          // Create the dispatcher thread
        dispatcher();
    });
}

void ThreadPool::schedule(const function<void(void)>& thunk) {
    {
        unique_lock<mutex> lock(queueLock);         // Lock the queue to safely add a task
        tasks.push(thunk);                          // Add the task to the queue
        {
            unique_lock<mutex> wait_lock(wait_mtx); // Lock the wait mutex
            activeTasks++;                          // Increment the count of active tasks
        }                  
    }

    dispatcher_cv.notify_one();                     // Notify the dispatcher that a new task is available
}

void ThreadPool::dispatcher() {
    while (true) {
        function<void(void)> task;

        {
            unique_lock<mutex> lock(queueLock);     // Lock the queue to safely access tasks
            dispatcher_cv.wait(lock, [this]() {     // Wait for tasks to be available
                return !tasks.empty() || done;
            });

            if (done && tasks.empty()) {             // If done and no tasks, exit
                return;
            }

            task = move(tasks.front());              // Get the next task
            tasks.pop();                             // Remove it from the queue
        }

        bool taskAssigned = false;                  // Flag to check if task is assigned

        while(!taskAssigned) {
            for (size_t i = 0; i < wts.size(); i++) { // Iterate through workers
                worker_t& worker = wts[i];

                unique_lock<mutex> lock(worker.mtx); // Lock the worker's mutex

                if (worker.available && !worker.hasTask) { // If worker is available and has no task
                    worker.thunk = move(task);       // Assign the task to the worker
                    worker.hasTask = true;           // Mark the worker as having a task
                    worker.available = false;        // Mark the worker as not available
                    taskAssigned = true;             // Task is assigned
                    worker.cv.notify_one();          // Notify the worker that a task is ready
                    break;                           // Break out of the loop since task is assigned
                }
            }

            if (!taskAssigned) {                    // If no worker was available
                this_thread::yield();               // Yield to allow other threads to run
            }
        }
    }
}

void ThreadPool::worker(int id) {
    worker_t& worker = wts[id];                      // Get the worker by ID

    while (true) {
        function<void(void)> task;

        {
            unique_lock<mutex> lock(worker.mtx);     // Lock the worker's mutex
            worker.cv.wait(lock, [&worker]() {       // Wait for a task to be assigned
                return worker.hasTask;
            });

            if (done && !worker.hasTask) { // If done and no task, exit
                return;
            }

            task = move(worker.thunk);               // Get the task assigned to the worker
            worker.hasTask = false;                  // Mark the worker as available
        }

        task();                                       // Execute the task

        {
            unique_lock<mutex> lock(worker.mtx);         // Lock the worker's mutex again
            worker.available = true;              // Mark the worker as available
        }
            
        {
            unique_lock<mutex> wait_lock(wait_mtx); // Lock the wait mutex
            activeTasks--;                          // Decrement the count of active tasks
            if (activeTasks == 0) {
                wait_cv.notify_all();              // Notify if all tasks are done
            }
        }
    }
}

void ThreadPool::wait() {
    unique_lock<mutex> lock(wait_mtx);              // Lock the wait mutex
    wait_cv.wait(lock, [this]() {                    // Wait until all tasks are done
        return activeTasks == 0;
    });
}

ThreadPool::~ThreadPool() {
    done = true;                                  // Set the done flag to true
    dispatcher_cv.notify_all();                   // Notify the dispatcher to exit

    for (auto& worker : wts) {                    // Join all worker threads
        worker.cv.notify_one();                // Notify each worker to exit if they are waiting
    }

    for (auto& worker : wts) {
        if (worker.ts.joinable()) {
            worker.ts.join();                     // Join the worker thread
        }
    }

    if (dt.joinable()) {
        dt.join();                                 // Join the dispatcher thread
    }
}
