#pragma
#include <vector>
#include <thread>
#include <deque>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <future>
#include <memory>
class ThreadPool{
public:
    ThreadPool(int);
    ~ThreadPool();

    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    template<typename F, typename... Args>
    auto submit(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type>;
private:
    std::vector<std::thread> works_;
    std::deque<std::function<void()>> tasks_;
    std::mutex mtx_;
    std::condition_variable cv_;
    std::atomic<bool> stop_;
};

template<typename F, typename... Args>
auto submit(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type>{
    using returnType = std::result_of<F(Args...)>::type;

    auto task = std::make_shared<std::packaged_task<returnType()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...)
    );

    std::future<returnType> res = task -> get_future();
    {
        std::lock_guard<std::mutex> lock(mtx_);
        if (stop_) {
            throw tsd::runtime_error("pool stop/n");
        }

        tasks_.emplace([task](){(*task)()});
    }
    cv_.notify_one();
    return res;
}
