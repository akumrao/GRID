

#ifndef dispatchqueue_hpp
#define dispatchqueue_hpp

#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <functional>
#include <string>

class DispatchQueue {
    typedef std::function<void(void)> fp_t;

public:
    DispatchQueue(std::string name, size_t threadCount = 1);
    ~DispatchQueue();

    // dispatch and copy
    void dispatch(const fp_t& op);
    // dispatch and move
    void dispatch(fp_t&& op);

    void removePending();

    // Deleted operations
    DispatchQueue(const DispatchQueue& rhs) = delete;
    DispatchQueue& operator=(const DispatchQueue& rhs) = delete;
    DispatchQueue(DispatchQueue&& rhs) = delete;
    DispatchQueue& operator=(DispatchQueue&& rhs) = delete;

private:
    std::string name;
    std::mutex lockMutex;
    std::vector<std::thread> threads;
    std::queue<fp_t> queue;
    std::condition_variable condition;
    bool quit = false;

    void dispatchThreadHandler(void);
};

#endif /* dispatchqueue_hpp */
