#pragma once
#include <mutex>
#include <condition_variable>

class semaphore {
private:
    std::mutex m;
    std::condition_variable cv;
    unsigned int count;
public:
    semaphore(const unsigned int &n)
        :count(n)
    {}
    void notify() {
        std::unique_lock<std::mutex> l(m);
        ++count;
        cv.notify_one();
    }
    void wait() {
        std::unique_lock<std::mutex> l(m);
        cv.wait(l, [this]{ return count != 0; });
        --count;
    }
};

class semaphore_controller {
    semaphore &s;
public:
    semaphore_controller(semaphore &s)
        :s{s}
    { s.wait(); }
    ~semaphore_controller()
    { s.notify(); }
};

