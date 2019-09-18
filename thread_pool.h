#ifndef _THREAD_POOL_H_
#define _THREAD_POOL_H_

#include <windows.h>
#include "task_queue.h"
#include <functional>
#include <vector>

namespace thread
{
    template<typename Task>
    class ThreadPool
    {
    public:
        ThreadPool()
        {}
        ~ThreadPool()
        {
        }

    public:
        bool begin(const std::function<void(Task&)>& on_task, int thread_num = 0)
        {
            is_end_ = false;
            thread_num_ = thread_num;
            task_func_ = on_task;

            if (!get_thread_count())
            {
                return false;
            }

            sleep_handle_ = ::CreateEvent(NULL, TRUE, FALSE, 0);
            exit_handle_ = ::CreateEvent(NULL, TRUE, FALSE, 0);
            if (NULL == sleep_handle_ || NULL == exit_handle_)
            {
                return false;
            }

            DWORD id;
            HANDLE thread = NULL;
            for (int i = 0; i < thread_num_; ++i)
            {
                thread = ::CreateThread(NULL, 0, thread_fun, (LPVOID)this, 0, &id);
                if (NULL == thread)
                {
                    return false;
                }
                thread_lst_.push_back(thread);
            }

            return true;
        }

        void end()
        {
            is_end_ = true;
            if (NULL != sleep_handle_)
            {
                SetEvent(sleep_handle_);
            }

            if (NULL != exit_handle_)
            {
                SetEvent(exit_handle_);
            }
            
            for (auto itor = thread_lst_.begin(); itor != thread_lst_.end(); ++itor)
            {
                if (NULL != *itor)
                {
                    if (WAIT_OBJECT_0 != WaitForSingleObject(*itor, 5000))
                    {
                        TerminateThread(*itor, -1);
                    }
                    CloseHandle(*itor);
                    *itor = NULL;
                }
            }

            thread_lst_.clear();
            task_queue_.clear();  
            if (NULL != sleep_handle_)
            {
                CloseHandle(sleep_handle_);
                sleep_handle_ = NULL;
            }

            if (NULL != exit_handle_)
            {
                CloseHandle(exit_handle_);
                exit_handle_ = NULL;
            }
        }

        bool post_task(const Task& task)
        {
            if (NULL == sleep_handle_)
            {
                return false;
            }

            task_queue_.post(task);
            SetEvent(sleep_handle_);
            return true;
        }

        int task_count() const
        {
            return task_queue_.task_count();
        }
    private:

        static DWORD WINAPI thread_fun(LPVOID hdl)
        {
            ThreadPool* p = (ThreadPool*)hdl;

            p->process();
            return 0;
        }

        void process()
        {
            if (nullptr == task_func_)
            {
                return;
            }

            Task task;
            do
            {
                if (WaitForSingleObject(exit_handle_, 0) == WAIT_OBJECT_0)
                {
                    break;
                }

                while (task_queue_.peek(task))
                {
                    task_func_(task);
                }

                if (!is_end_)
                {
                    ResetEvent(sleep_handle_);
                }                

                WaitForSingleObject(sleep_handle_, INFINITE);
            } while (1);
        }


        bool get_thread_count()
        {
            if (thread_num_ <= 0)
            {
                SYSTEM_INFO sysInfo;
                GetSystemInfo(&sysInfo);
                thread_num_ = sysInfo.dwNumberOfProcessors;
            }

            if (thread_num_ <= 0)
            {
                return false;
            }

            return true;
        }

    private:
        int thread_num_; //线程个数
        bool is_end_;

        HANDLE sleep_handle_;
        HANDLE exit_handle_;

        mutable queue::TaskQueue<Task> task_queue_;
        std::vector<HANDLE>      thread_lst_;  //线程列表


        std::function<void(Task&)>  task_func_;
    };
} //end namespace thread


#endif  //_THREAD_POOL_H_


