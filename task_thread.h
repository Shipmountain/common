
#ifndef _TASK_THREAD_H_
#define _TASK_THREAD_H_

#include "thread.h"
#include "task_queue.h"
#include <functional>

namespace thread
{

template<typename Task>
class TaskThread
{
public:
	TaskThread()
	{
	}

	~TaskThread()
	{
	}

	bool begin(const std::function<void(Task&)>& on_task)
	{
		m_task_func = on_task;
		m_thread.init(ThreadFunc, this, true, (float)-1);
		m_thread.start();
		return true;
	}

	void end()
	{
		m_thread.stop();
	}

	void post_task(const Task& task)
	{
		m_TaskQueue.post(task);
		m_thread.reset();
	}

	int task_count() const
	{
		return m_TaskQueue.task_count();
	}

private:

    std::function<void(Task&)> get_task_fun()
    {
        return m_task_func;
    }

    queue::TaskQueue<Task>& get_task_queue()
    {
        return m_TaskQueue;
    }


	static void* ThreadFunc(void* pParam)
	{
		TaskThread<Task>* pImpl = (TaskThread<Task> *)pParam;
		if (!pImpl)
		{
			return 0;
		}

        queue::TaskQueue<Task>& _TaskQueue = pImpl->get_task_queue();
		std::function<void(Task&)> _TaskFunc = pImpl->get_task_fun();

		Task task;
		while (_TaskQueue.peek(task))
		{
			if (_TaskFunc != 0)
			{
				_TaskFunc(task);
			}
		}

		return pImpl;
	}

private:
	mutable queue::TaskQueue<Task> m_TaskQueue;
	Thread m_thread;
	std::function<void(Task&)> m_task_func;
};

}  // namespace thread

#endif  // _TASK_THREAD_H_
