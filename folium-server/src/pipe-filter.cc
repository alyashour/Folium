#include <iostream>
#include <queue>
#include <functional>
#include <vector>
#include <mutex>
#include <thread>
#include <condition_variable>

// The Pipe class represents a thread-safe queue for communication between threads.
class Pipe {
private:
    std::queue<int> dataQueue; // Queue to store data
    std::mutex mtx; // Mutex to protect access to the queue
    std::condition_variable cv; // Condition variable for synchronization

public:
    // Sends data into the pipe (adds it to the queue).
    void send(int data) {
        std::unique_lock<std::mutex> lock(mtx); // Lock the mutex
        dataQueue.push(data); // Add data to the queue
        cv.notify_one(); // Notify one waiting thread
    }

    // Receives data from the pipe (removes it from the queue).
    int receive() {
        std::unique_lock<std::mutex> lock(mtx); // Lock the mutex
        // Wait until the queue is not empty
        cv.wait(lock, [this]() { return !dataQueue.empty(); });
        int data = dataQueue.front(); // Get the front element
        dataQueue.pop(); // Remove the front element
        return data; // Return the data
    }
};

// The Filter class represents a processing unit that applies a function to data.
class Filter {
private:
    std::function<int(int)> processFunction;
    std::thread workerThread;
    bool running;
    Pipe* inputPipe;
    Pipe* outputPipe;

public:
    // Constructor now takes pipes as parameters
    explicit Filter(std::function<int(int)> func, Pipe* in, Pipe* out) 
        : processFunction(std::move(func)), inputPipe(in), outputPipe(out), running(true) {
        // Start the worker thread immediately
        workerThread = std::thread(&Filter::run, this);
    }

    // Destructor to ensure clean shutdown
    ~Filter() {
        stop();
    }

    // Main processing loop
    void run() {
        while (running) {
            int data = inputPipe->receive();
            int processedData = processFunction(data);
            outputPipe->send(processedData);
        }
    }

    // Method to stop the filter
    void stop() {
        running = false;
        if (workerThread.joinable()) {
            workerThread.join();
        }
    }

    // Process method now private as it's handled internally
private:
    int process(int data) {
        return processFunction(data);
    }
};