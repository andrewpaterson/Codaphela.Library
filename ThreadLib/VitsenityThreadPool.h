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

#ifndef __THREAD_POOL_H__
#define __THREAD_POOL_H__

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
        std::unique_lock<std::mutex> lock(this->mutex);
        this->q.push(value);
        return true;
    }

    // deletes the retrieved element, do not use for non integral types
    bool Pop(T& v)
    {
        std::unique_lock<std::mutex> lock(this->mutex);
        if (this->q.empty())
            return false;
        v = this->q.front();
        this->q.pop();
        return true;
    }

    bool IsEmpty()
    {
        std::unique_lock<std::mutex> lock(this->mutex);
        return this->q.empty();
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
        this->Clear();
    }

    void Init(int iNumThreads)
    {
        this->Clear();
        this->Resize(iNumThreads);
    }

    void Init(void)
    {
        int				iNumHardwareTheads;

        this->Clear();

        iNumHardwareTheads = std::thread::hardware_concurrency();
        this->Resize(iNumHardwareTheads);
    }


    // the destructor waits for all the functions in the queue to be finished
    void Kill(void)
    {
        Stop();
    }

    void Clear()
    {
        this->nWaiting = 0;
        this->isStop = false;
        this->isDone = false;
    }

    // get the number of running threads in the pool
    int Size()
    {
        return static_cast<int>(this->threads.size());
    }


    // number of idle threads
    int NumberOfIdleThreads()
    {
        return this->nWaiting;
    }


    std::thread& getThread(int i)
    {
        return *this->threads[i];
    }

    // change the number of threads in the pool
    // should be called from one thread, otherwise be careful to not interleave, also with this->stop()
    // nThreads must be >= 0
    void Resize(int nThreads)
    {
        if (!this->isStop && !this->isDone)
        {
            int oldNThreads = static_cast<int>(this->threads.size());
            if (oldNThreads <= nThreads)
            {
                // if the number of threads is increased
                this->threads.resize(nThreads);
                this->flags.resize(nThreads);

                for (int i = oldNThreads; i < nThreads; ++i) {
                    this->flags[i] = std::make_shared<std::atomic<bool>>(false);
                    this->SetThread(i);
                }
            }
            else
            {
                // the number of threads is decreased
                for (int i = oldNThreads - 1; i >= nThreads; --i) {
                    *this->flags[i] = true;  // this thread will finish
                    this->threads[i]->detach();
                }
                {
                    // stop the detached threads that were waiting
                    std::unique_lock<std::mutex> lock(this->mutex);
                    this->cv.notify_all();
                }
                this->threads.resize(nThreads);  // safe to delete because the threads are detached
                this->flags.resize(nThreads);  // safe to delete because the threads have copies of shared_ptr of the flags, not originals
            }
        }
    }


    // empty the queue
    void ClearQueue()
    {
        std::function<void(int id)>* _f;
        while (this->q.Pop(_f))
        {
            delete _f; // empty the queue
        }
    }


    // pops a functional wrapper to the original function
    std::function<void(int)> Pop()
    {
        std::function<void(int id)>* _f = nullptr;

        this->q.Pop(_f);
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
            if (this->isStop)
            {
                return;
            }
            this->isStop = true;
            for (int i = 0, n = this->Size(); i < n; ++i)
            {
                *this->flags[i] = true;  // command the threads to stop
            }
            this->ClearQueue();  // empty the queue
        }
        else
        {
            if (this->isDone || this->isStop)
            {
                return;
            }
            this->isDone = true;  // give the waiting threads a command to finish
        }

        {
            std::unique_lock<std::mutex> lock(this->mutex);
            this->cv.notify_all();  // stop all waiting threads
        }

        for (int i = 0; i < static_cast<int>(this->threads.size()); ++i)
        {
            // wait for the computing threads to finish
            if (this->threads[i]->joinable())
            {
                this->threads[i]->join();
            }
        }
        // if there were no threads in the pool but some functors in the queue, the functors are not deleted by the threads
        // therefore delete them here
        this->ClearQueue();
        this->threads.clear();
        this->flags.clear();
    }


    template<typename F, typename... Rest>
    auto Push(F&& f, Rest&&... rest) ->std::future<decltype(f(0, rest...))>
    {
        auto pck = std::make_shared<std::packaged_task<decltype(f(0, rest...))(int)>>(std::bind(std::forward<F>(f), std::placeholders::_1, std::forward<Rest>(rest)...));
        auto _f = new std::function<void(int id)>([pck](int id)
        {
            (*pck)(id);
        });

        this->q.Push(_f);
        std::unique_lock<std::mutex> lock(this->mutex);
        this->cv.notify_one();
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
        this->q.Push(_f);
        std::unique_lock<std::mutex> lock(this->mutex);
        this->cv.notify_one();
        return pck->get_future();
    }

    void SetThread(int i)
    {
        std::shared_ptr<std::atomic<bool>> flag(this->flags[i]); // a copy of the shared ptr to the flag
        auto f = [this, i, flag/* a copy of the shared ptr to the flag */]()
        {
            std::atomic<bool>& _flag = *flag;
            std::function<void(int id)>* _f;
            bool isPop = this->q.Pop(_f);
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
                        isPop = this->q.Pop(_f);
                    }
                }
                // the queue is empty here, wait for the next command
                std::unique_lock<std::mutex> lock(this->mutex);
                ++this->nWaiting;
                this->cv.wait(lock, [this, &_f, &isPop, &_flag]() { isPop = this->q.Pop(_f); return isPop || this->isDone || _flag; });
                --this->nWaiting;
                if (!isPop)
                {
                    return;  // if the queue is empty and this->isDone == true or *flag then return
                }
            }
        };

        this->threads[i].reset(new std::thread(f)); // compiler may not support std::make_unique()
    }
};


#endif // __THREAD_POOL_H__

