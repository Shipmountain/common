
#ifndef _SINGLETON_H_
#define _SINGLETON_H_

#include "lock.h"

namespace comm
{

	template <class T>
	class Singleton
	{
	private:
		Singleton() {}
		Singleton(const Singleton&) {}
		Singleton& operator =(const Singleton&){}
		virtual ~Singleton(){}

	public:
		static T& instance();
		static void release();

	private:
        static lock::Mutex m_mutex;
		static T* m_pInstance;
	};


	template <class T>
    lock::Mutex Singleton<T>::m_mutex;

	template <class T>
	T* Singleton<T>::m_pInstance = 0;

	template <class T>
	T& Singleton<T>::instance()
	{
		if (0 == m_pInstance)
		{
            lock::Lock lock(&m_mutex);
			if (m_pInstance == NULL)
				m_pInstance = new T;
		}
		return *m_pInstance;
	}

	template <class T>
	void Singleton<T>::release()
	{
		if (0 != m_pInstance)
		{
            lock::Lock lock(&m_mutex);
			if (0 != m_pInstance)
			{
				delete m_pInstance;
				m_pInstance = 0;
			}
		}
	}

}

#endif  // _SINGLETON_H_
