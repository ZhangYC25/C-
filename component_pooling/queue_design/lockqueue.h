#ifndef LOCKQUEUE_H
#define LOCKQUEUE_H

#include <deque>
#include <mutex>

template <class T,typename StorageType =std::deque<T>>
class LockedQueue{
    std::mutex _lock;
    StorageType _queue;
    volatile bool _canceled;
public:
    LockedQueue ():_canceled (false) {}
    virtual ~LockedQueue () {}
    //这里参数为什么是T
    void add (const T& item){
        lock ();
        _queue.push_back (item);
        unlock ();
    }

    //这是在干什么？？？不是很清楚里面的这些mutexAPI
    //std::lock_guard<std::mutex> lock(_lock);看不懂
    template<class Iterator>
    void readd (Iterator begin, Iterator end){
        std::lock_guard<std::mutex> lock (_lock);
        _queue.insert (_queue.begin(), begin, end);
    }

    bool next (T& result) {
        std::lock_guard<std::mutex> lock(_lock);
        if (_queue.empty ()) return false;
        result = _queue.front ();
        _queue.pop_front ();
        return true;
    }
    template<class Checker>
    bool next (T& result, Checker& check) {
        std::lock_guard<std::mutex> lock(_lock);
        if (_queue.empty()) return false;
        result = _queue.front();
        if (!check.Process(result)) return false;
        _queue.pop_front();
        return true;
    }

    T& peek (bool autonlock = false) {
        lock ();
        T& result = _queue.front();
        if (autonlock) unlock ();
        return result;
    }

    void cancel () {
        std::lock_guard<std::mutex> lock(_lock);
        _canceled = true;
    }
    bool cancelled () {
        std::lock_guard<std::mutex> lock(_lock);
        return _canceled;
    }


    void lock () {this -> _lock.lock();}
    void unlock () {this -> _lock.unlock();}
    void pop_front () {
        std::lock_guard<std::mutex> lock(_lock);
        _queue.pop_front;
    }

    bool empty () {
        std::lock_guard<std::mutex> lock(_lock);
        return _queue.empty();
    }
};

#endif