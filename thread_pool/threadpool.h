#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <vector>
#include <thread>
#include <functional>
#include <mutex>
#include <queue>
#include <type_traits>
#include <future>
#include <utility>
#include <memory>

namespace pool {
    class ThreadPool {
    public:
        explicit ThreadPool(size_t);
        template <typename F, typename... Args>
        auto enqueue(F&& f, Args&&... args)
             -> std::future<typename std::result_of<F(Args...)>::type>;
        ~ThreadPool();
    private:
        std::mutex queue_mutex;
        std::vector<std::thread> workers;
        std::queue<std::function<void()>> tasks;
        std::condition_variable condition;
        bool stop;
    };

    inline ThreadPool::ThreadPool(size_t threads): stop(false)
    {
        for (auto i = 0; i < threads; i++) {
            workers.emplace_back(
                [this] {
                    while (true) {
                        std::function<void()> task;
                        {
                            std::unique_lock<std::mutex> lock(this -> queue_mutex);
                            this -> condition.wait(lock, 
                                [this] { return this -> stop || !this -> tasks.empty(); });
                            if (this -> stop && this -> tasks.empty())
                                return;
                            task = std::move(this -> tasks.front());
                            this -> tasks.pop();
                        }
                        task();
                    }
                }
            );
        }
    }

    template <typename F, typename... Args>
    auto ThreadPool::enqueue(F&& f, Args&&... args)
        -> std::future<typename std::result_of<F(Args...)>::type>
    {
        using return_type = typename std::result_of<F(Args...)>::type;
        
        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );

        auto ret_future = task -> get_future();
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            if (stop)
                throw std::runtime_error("enqueue on stopped threadpool");
            tasks.emplace(
                [task] { (*task)(); }
            );
            condition.notify_one();
        }
        return ret_future;
    }

    ThreadPool::~ThreadPool()
    {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            stop = true;
        }
        condition.notify_all();
        for (auto &worker:workers)
            worker.join();
    }
}
#endif // THREAD_POOL_H end here