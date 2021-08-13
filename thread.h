#ifndef _THREAD_H_
#define _THREAD_H_

#include <string.h>
#include <windows.h>
#include <process.h>
typedef void* (*thread_func)(void*);

namespace thread
{
    class Thread
    {
    public:

		Thread():is_start_(false),thread_handle_(NULL)
			,exit_event_(NULL),timer_event_(NULL),thread_func_(NULL),callback_param_(NULL),timer_(0),loop_(false){
		}


		Thread(thread_func func, void* hdl, bool bLoop = true, float wait = 5)
            :is_start_(false),thread_handle_(NULL)
        {
			init(func,hdl,bLoop,wait);
        }

        ~Thread()
        {
            stop();
			uninit();
        }

		void init(thread_func func, void* cb_param, bool loop = false, unsigned timer = 0)
		{
			thread_func_ = func;
            callback_param_ = cb_param;
            timer_ = timer;
			loop_ = loop;

			exit_event_ = ::CreateEvent(NULL, FALSE, 0, 0);
            if (loop && 0 != timer)
            {
                timer_event_ = ::CreateEvent(NULL, FALSE, 0, 0);
            }			
		}

		void uninit()
		{
            if (NULL != exit_event_)
            {
                ::CloseHandle(exit_event_);
                exit_event_ = NULL;
            }
			
            if (NULL != timer_event_)
            {
                ::CloseHandle(timer_event_);
                timer_event_ = NULL;
            }			
		}

		int set_affinity(DWORD_PTR cpu_mask)
		{
			if (!thread_handle_)
				return FALSE;  
	
			DWORD_PTR ret = ::SetThreadAffinityMask(thread_handle_, cpu_mask);
			return !ret;
		}

		void set_period(int timer)
		{
            timer_ = timer;
		}

        bool start()
        {
            stop();
            DWORD id;
            thread_handle_ = ::CreateThread(NULL, NULL, work_proc, (LPVOID)this, 0, &id);
            if(thread_handle_ == NULL)
            {
                return false;
            }

            is_start_ = true;
            return true;
        }

		void reset()
		{
            if (NULL != timer_event_)
            {
                ::SetEvent(timer_event_);
            }
		}

        void stop()
        {
            if(!is_start_ || !thread_handle_) return;
            ::SetEvent(exit_event_);
            if (NULL != timer_event_) ::SetEvent(timer_event_);
            if(::WaitForSingleObject(thread_handle_,5000) != WAIT_OBJECT_0)
            {
                ::TerminateThread(thread_handle_, -1);
            }
            ::CloseHandle(thread_handle_);

            thread_handle_ = NULL;
            is_start_ = false;
        }
protected:
        static DWORD WINAPI work_proc(LPVOID hdl)
        {
            Thread* p = (Thread*)hdl;

            p->proc();
            return 0;
        }

        void proc()
        {
            if(!thread_func_)
                return;

            do
            {
                if(::WaitForSingleObject(exit_event_,0) == WAIT_OBJECT_0)
                {
                    break;
                }

                thread_func_(callback_param_);

                if (NULL != timer_event_)
                {
                    ::WaitForSingleObject(timer_event_, timer_);
                }
            }
            while (loop_);
        }

	private:
		thread_func thread_func_;
		void*       callback_param_;
		HANDLE      thread_handle_;
        HANDLE      exit_event_;
        HANDLE      timer_event_;
		unsigned    timer_;
		bool        loop_;
		bool        is_start_;
    };
}

#endif //_THREAD_H_
