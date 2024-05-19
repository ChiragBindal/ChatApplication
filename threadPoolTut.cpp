#include<iostream>
#include<vector>
#include<thread>
#include<queue>
#include<functional>
#include<mutex>
#include<condition_variable>
#include<future>

using namespace std;
class ThreadPool{
    private : 
    int m_threads;
    vector<thread> threads;
    queue<function<void()>> tasks;
    mutex mx;
    condition_variable cv;
    bool stop;

    public : 
    explicit ThreadPool(int numThreads) : m_threads(numThreads) , stop(false){
        for(int i=0;i<m_threads;++i){
            threads.emplace_back([this]{
                function<void()> task;
                while(1){
                    unique_lock<mutex> lock(mx);
                    cv.wait(lock ,[this]{
                        return !tasks.empty() || stop;
                    });
                    if(stop) return ;
                    task = std::move(tasks.front());
                    tasks.pop();

                    lock.unlock();
                    task();
                }
            });
        }
    }
    ~ThreadPool(){
        unique_lock<mutex> lock(mx);
        stop = true;
        lock.unlock();
        cv.notify_all();
        for(auto& th : threads){
            th.join();
        }
    }
    template< class F , class ... Args> 
    auto ExecuteTask(F&& f , Args&& ... args) -> future<decltype(f(args...))>{
        
        using return_type = decltype(f(args...));
        auto task = make_shared<packaged_task<return_type()>>(bind(std::forward<F>(f) , std::forward<Args>(args)...));
        future<return_type> res = task->get_future();

        unique_lock<mutex> lock(mx);
        tasks.emplace([task]() -> void{
            (*task)();
        });
        lock.unlock();
        cv.notify_one();
        return res;
    }
};
int func(int a){
    this_thread::sleep_for(chrono::seconds(2));
    cout<<"this is the time taking function"<<endl;
    return a*a;
}
