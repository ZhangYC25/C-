#include "threadpool.h"

ThreadPool::ThreadPool(int poolsize) : stop_(false){
    int i = 0;
    for (i;i < poolsize; ++i){
        works_.emplace_back([this](){
            while (true)
            {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> mtx_;
                    cv_.wait(mtx_,[this]{
                        stop_ || !tasks_.empty();
                    });
                    if (stop_ && tasks_.empty()) {
                        return;
                    }
                    task = std::move(tasks_.front());
                    tasks_.pop_front();
                }
                task();
            }
            
        });
    }
}

ThreadPool::~ThreadPool(){
    {
        std::lock_guard<std::mutex> lock(mtx_);
        stop_ = true;
    }

    cv_.notify_all();

    for (std::thread& th : works_){
        if(th.joinable()){
            th.join();
        }
    }
}