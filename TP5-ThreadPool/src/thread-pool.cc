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

    dispatcher_ready.signal();                     // Notify the dispatcher that a new task is available
}

void ThreadPool::dispatcher() {
    while(true) {
        dispatcher_ready.wait();                // Wait for a task to be available

        {
            unique_lock<mutex> lock(queueLock); // Lock the queue to safely check tasks
            if (done && tasks.empty()) break;   // If done and no tasks, exit the loop
            if (tasks.empty()) continue;        // If no tasks, continue waiting
        }

        function<void(void)> task;

        {
            unique_lock<mutex> lock(queueLock); // Lock the queue to safely access tasks
            if (tasks.empty()) continue;         // If no tasks, continue waiting

            task = move(tasks.front());          // Get the next task
            tasks.pop();                          // Remove the task from the queue
        }

        bool taskAssigned = false;

        while (!taskAssigned) {
            for (worker_t& worker : wts) { // Iterate through workers
                unique_lock<mutex> lock(worker.mtx); // Lock the worker's mutex
                if (worker.available && !worker.hasTask) { // If worker is available and has no task
                    worker.thunk = move(task); // Assign the task to the worker
                    worker.hasTask = true;     // Mark the worker as having a task
                    worker.available = false;  // Mark the worker as not available
                    worker.ready.signal(); // Signal the worker that a task is ready
                    taskAssigned = true;      // Mark the task as assigned
                    break;                   // Exit the loop as the task is assigned
                }
            }
        }
    }
}

void ThreadPool::worker(int id) {
    worker_t& worker = wts[id];                      // Get the worker by ID

    while (true) {
        worker.ready.wait();                         // Wait for a task to be ready

        if (done && !worker.hasTask) break;          // If done and no task, exit the loop

        function<void(void)> task;

        {
            unique_lock<mutex> lock(worker.mtx);    // Lock the worker's mutex
            if (!worker.hasTask) continue;          // If no task, continue waiting
            task = move(worker.thunk);              // Move the task to execute
            worker.hasTask = false;                 // Mark the worker as not having a task
        }

        task();                                      // Execute the task

        {
            unique_lock<mutex> lock(wait_mtx);    // Lock the worker's mutex
            activeTasks--;                        // Decrement the count of active tasks
            if (activeTasks == 0) {               // If no active tasks left
                wait_sem.signal();                  // Notify the wait condition
            }
        }
    }
}

void ThreadPool::wait() {
    unique_lock<mutex> lock(wait_mtx);              // Lock the wait mutex
    if (activeTasks > 0) {                          // If there are active tasks
        wait_sem.wait();                            // Wait until all tasks are done
    }
}

ThreadPool::~ThreadPool() {
    done = true;                                    // Set the done flag to true
    dispatcher_ready.signal();                      // Notify the dispatcher to exit
    
    for (auto& worker : wts) {
        if (worker.ts.joinable()) {
            worker.ts.join();                       // Join each worker thread
        }
    }

    if (dt.joinable()) {
        dt.join();                                  // Join the dispatcher thread
    }
}
