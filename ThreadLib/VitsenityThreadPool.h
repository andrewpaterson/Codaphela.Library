/*********************************************************
*
*  Copyright (C) 2014 by Vitaliy Vitsentiy
*
*  Licensed under the Apache License, Version 2.0 (the "License");
*  you may not use this file except in compliance with the License.
*  You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
*  Unless required by applicable law or agreed to in writing, software
*  distributed under the License is distributed on an "AS IS" BASIS,
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*  See the License for the specific language governing permissions and
*  limitations under the License.
*
*********************************************************/

//https://github.com/vit-vit/CTPL/blob/master/ctpl.h

#ifndef __VISENITY_THREAD_POOL_H__
#define __VISENITY_THREAD_POOL_H__

#include <functional>
#include <thread>
#include <atomic>
#include <vector>
#include <memory>
#include <exception>
#include <future>
#include <mutex>
#include <queue>



// thread pool to run user's functors with signature
//      ret func(int id, other_params)
// where id is the index of the thread that runs the functor
// ret is some return type
template <typename T>
class CQueue
{
private:
    std::queue<T> q;
    std::mutex mutex;

public:
    bool Push(T const& value)
    {
        std::unique_lock<std::mutex> lock(mutex);
        q.push(value);
        return true;
    }

    // deletes the retrieved element, do not use for non integral types
    bool Pop(T& v)
    {
        std::unique_lock<std::mutex> lock(mutex);
        if (q.empty())
            return false;
        v = q.front();
        q.pop();
        return true;
    }

    bool IsEmpty()
    {
        std::unique_lock<std::mutex> lock(mutex);
        return q.empty();
    }
};


class CVitsenityThreadPool 
{
private:
    std::vector<std::unique_ptr<std::thread>> threads;
    std::vector<std::shared_ptr<std::atomic<bool>>> flags;
    CQueue<std::function<void(int id)>*> q;
    std::atomic<bool> isDone;
    std::atomic<bool> isStop;
    std::atomic<int> nWaiting;  // how many threads are waiting

    std::mutex mutex;
    std::condition_variable cv;

private:
    // deleted
    CVitsenityThreadPool(const CVitsenityThreadPool&);// = delete;
    CVitsenityThreadPool(CVitsenityThreadPool&&);// = delete;
    CVitsenityThreadPool& operator=(const CVitsenityThreadPool&);// = delete;
    CVitsenityThreadPool& operator=(CVitsenityThreadPool&&);// = delete;

public:
    CVitsenityThreadPool()
    {
        Clear();
    }

    void Init(int iNumThreads)
    {
        Clear();
        Resize(iNumThreads);
    }

    void Init(void)
    {
        int				iNumHardwareTheads;

        Clear();

        iNumHardwareTheads = std::thread::hardware_concurrency();
        Resize(iNumHardwareTheads);
    }


    // the destructor waits for all the functions in the queue to be finished
    void Kill(void)
    {
        Stop();
    }

    void Clear()
    {
        nWaiting = 0;
        isStop = false;
        isDone = false;
    }

    // get the number of running threads in the pool
    int Size()
    {
        return static_cast<int>(threads.size());
    }


    // number of idle threads
    int NumberOfIdleThreads()
    {
        return nWaiting;
    }


    std::thread& getThread(int i)
    {
        return *threads[i];
    }

    // change the number of threads in the pool
    // should be called from one thread, otherwise be careful to not interleave, also with stop()
    // nThreads must be >= 0
    void Resize(int nThreads)
    {
        if (!isStop && !isDone)
        {
            int oldNThreads = static_cast<int>(threads.size());
            if (oldNThreads <= nThreads)
            {
                // if the number of threads is increased
                threads.resize(nThreads);
                flags.resize(nThreads);

                for (int i = oldNThreads; i < nThreads; ++i) {
                    flags[i] = std::make_shared<std::atomic<bool>>(false);
                    SetThread(i);
                }
            }
            else
            {
                // the number of threads is decreased
                for (int i = oldNThreads - 1; i >= nThreads; --i) {
                    *flags[i] = true;  // this thread will finish
                    threads[i]->detach();
                }
                {
                    // stop the detached threads that were waiting
                    std::unique_lock<std::mutex> lock(mutex);
                    cv.notify_all();
                }
                threads.resize(nThreads);  // safe to delete because the threads are detached
                flags.resize(nThreads);  // safe to delete because the threads have copies of shared_ptr of the flags, not originals
            }
        }
    }


    // empty the queue
    void ClearQueue()
    {
        std::function<void(int id)>* _f;
        while (q.Pop(_f))
        {
            delete _f; // empty the queue
        }
    }


    // pops a functional wrapper to the original function
    std::function<void(int)> Pop()
    {
        std::function<void(int id)>* _f = nullptr;

        q.Pop(_f);
        std::unique_ptr<std::function<void(int id)>> func(_f); // at return, delete the function even if an exception occurred
        std::function<void(int)> f;
        if (_f)
        {
            f = *_f;
        }
        return f;
    }

    // wait for all computing threads to finish and stop all threads
    // may be called asynchronously to not pause the calling thread while waiting
    // if isWait == true, all the functions in the queue are run, otherwise the queue is cleared without running the functions
    void Stop(bool isWait = false)
    {
        if (!isWait)
        {
            if (isStop)
            {
                return;
            }
            isStop = true;
            for (int i = 0, n = Size(); i < n; ++i)
            {
                *flags[i] = true;  // command the threads to stop
            }
            ClearQueue();  // empty the queue
        }
        else
        {
            if (isDone || isStop)
            {
                return;
            }
            isDone = true;  // give the waiting threads a command to finish
        }

        {
            std::unique_lock<std::mutex> lock(mutex);
            cv.notify_all();  // stop all waiting threads
        }

        for (int i = 0; i < static_cast<int>(threads.size()); ++i)
        {
            // wait for the computing threads to finish
            if (threads[i]->joinable())
            {
                threads[i]->join();
            }
        }
        // if there were no threads in the pool but some functors in the queue, the functors are not deleted by the threads
        // therefore delete them here
        ClearQueue();
        threads.clear();
        flags.clear();
    }


    template<typename F, typename... Rest>
    auto Push(F&& f, Rest&&... rest) ->std::future<decltype(f(0, rest...))>
    {
        auto pck = std::make_shared<std::packaged_task<decltype(f(0, rest...))(int)>>(std::bind(std::forward<F>(f), std::placeholders::_1, std::forward<Rest>(rest)...));
        auto _f = new std::function<void(int id)>([pck](int id)
        {
            (*pck)(id);
        });

        q.Push(_f);
        std::unique_lock<std::mutex> lock(mutex);
        cv.notify_one();
        return pck->get_future();
    }

    // run the user's function that excepts argument int - id of the running thread. returned value is templatized
    // operator returns std::future, where the user can get the result and rethrow the caught exceptions
    template<typename F>
    auto Push(F&& f) ->std::future<decltype(f(0))>
    {
        auto pck = std::make_shared<std::packaged_task<decltype(f(0))(int)>>(std::forward<F>(f));
        auto _f = new std::function<void(int id)>([pck](int id)
            {
                (*pck)(id);
            });
        q.Push(_f);
        std::unique_lock<std::mutex> lock(mutex);
        cv.notify_one();
        return pck->get_future();
    }

    void SetThread(int i)
    {
        std::shared_ptr<std::atomic<bool>> flag(flags[i]); // a copy of the shared ptr to the flag
        auto f = [this, i, flag/* a copy of the shared ptr to the flag */]()
        {
            std::atomic<bool>& _flag = *flag;
            std::function<void(int id)>* _f;
            bool isPop = q.Pop(_f);
            while (true)
            {
                while (isPop)
                {  // if there is anything in the queue
                    std::unique_ptr<std::function<void(int id)>> func(_f); // at return, delete the function even if an exception occurred
                    (*_f)(i);
                    if (_flag)
                    {
                        return;  // the thread is wanted to stop, return even if the queue is not empty yet
                    }
                    else
                    {
                        isPop = q.Pop(_f);
                    }
                }
                // the queue is empty here, wait for the next command
                std::unique_lock<std::mutex> lock(mutex);
                ++nWaiting;
                cv.wait(lock, [this, &_f, &isPop, &_flag]() { isPop = q.Pop(_f); return isPop || isDone || _flag; });
                --nWaiting;
                if (!isPop)
                {
                    return;  // if the queue is empty and isDone == true or *flag then return
                }
            }
        };

        threads[i].reset(new std::thread(f)); // compiler may not support std::make_unique()
    }
};


#endif // !__VISENITY_THREAD_POOL_H__

