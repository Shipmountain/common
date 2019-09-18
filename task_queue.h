#ifndef _TASK_QUEUE_H_
#define _TASK_QUEUE_H_

#include <deque>
#include "lock.h"

namespace queue
{

template<typename Task>
class TaskQueue
{
public:
	void post(const Task& task)
	{
		lock::Lock lock(&m_mutex);
		m_task.push_back(task);
	}

	bool peek(Task& task)
	{
		lock::Lock lock(&m_mutex);
		if (!m_task.empty())
		{
			task = m_task.front();
			m_task.pop_front();
			return true;
		}

		return false;
	}

	bool is_empty() const
	{
		lock::Lock lock(&m_mutex);
		return m_task.empty();
	}

	int task_count() const
	{
		lock::Lock lock(&m_mutex);
		return m_task.size();
	}

    void clear()
    {
        lock::Lock lock(&m_mutex);
        m_task.clear();
    }

private:
    mutable lock::Mutex m_mutex;
	std::deque<Task> m_task;
};

}  // namespace comm

#endif  // _TASK_QUEUE_H_
