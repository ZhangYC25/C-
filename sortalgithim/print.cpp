#include <mutex>
#include <iostream>
#include <condition_variable>
#include <thread>
int num = 0;
std::mutex mtx;
std::condition_variable cv;

#if 0
bool flag = false; //false 打印 奇数，true 打印 偶数

void print_odd(){
    while(num <= 100){
        {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock,[&](){return !flag;});
            if (num > 100) break;

            std::cout<<"1: "<<num<<std::endl;;
            flag = true;
            num++;
            cv.notify_one();
        }
    }
}

void print_even(){
    while(num <= 100){
        {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock,[&](){return flag;});
            if (num > 100) break;

            std::cout<<"2: "<<num<<std::endl;
            flag = false;
            num++;
            cv.notify_one();
        }
    }
}

#else 

int turn = 0;
void print_0() {
    while (true) {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [](){ return turn == 0 || num > 100; });
        
        if (num > 100) break;

        std::cout << "0: " << num << std::endl;
        num++;
        turn = 1; // 下一个轮到 print_1
        cv.notify_all(); // 唤醒所有，让正确的那个继续
    }
}

void print_1() {
    while (true) {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [](){ return turn == 1 || num > 100; });
        
        if (num > 100) break;

        std::cout << "1: " << num << std::endl;
        num++;
        turn = 2; // 下一个轮到 print_2
        cv.notify_all();
    }
}

void print_2() {
    while (true) {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [](){ return turn == 2 || num > 100; });
        
        if (num > 100) break;

        std::cout << "2: " << num << std::endl;
        num++;
        turn = 0; // 下一个轮到 print_0
        cv.notify_all();
    }
}


#endif

int main(){
    std::thread t1(print_0);
    std::thread t2(print_1);
    std::thread t3(print_2);

    t1.join();
    t2.join();
    t3.join();
}