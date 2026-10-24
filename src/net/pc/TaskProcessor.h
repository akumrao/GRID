#include <iostream>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <memory>

class TaskProcessor {
private:
    std::queue<std::function<void()>> task_queue;
    std::mutex queue_mutex;
    std::condition_variable cv;
    std::thread worker_thread;
    bool stop_requested = false;

    void worker_loop() {
        while (true) {
            std::function<void()> task;
            {
                std::unique_lock<std::mutex> lock(queue_mutex);
                cv.wait(lock, [this] { return stop_requested || !task_queue.empty(); });
                
                if (stop_requested && task_queue.empty()) return;
                
                task = std::move(task_queue.front());
                task_queue.pop();
            }
            task(); // Execute the deferred function call
        }
    }

public:
    TaskProcessor() {
        worker_thread = std::thread(&TaskProcessor::worker_loop, this);
    }

    ~TaskProcessor() {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            stop_requested = true;
        }
        cv.notify_one();
        if (worker_thread.joinable()) {
            worker_thread.join();
        }
    }

    // Variadic template allows passing any function/method and its parameters
    template <typename Callable, typename... Args>
    void enqueue(Callable&& callable, Args&&... args) {
        // Bind the callable and arguments together into a std::function<void()>
        auto bound_task = std::bind(std::forward<Callable>(callable), std::forward<Args>(args)...);

        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            task_queue.push(bound_task);
        }
        cv.notify_one();
    }
};

