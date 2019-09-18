#ifndef _LOCK_H_
#define _LOCK_H_

#include <windows.h>
namespace lock
{
    class Lock;
	class TimeLock;
    class Mutex
    {
    friend class Lock;
	friend class TimeLock;
    public:
        Mutex(){
            InitializeCriticalSection(&m_crit);
        }
        ~Mutex(){
            DeleteCriticalSection(&m_crit);
        }
    private:
        CRITICAL_SECTION m_crit;
    };
	
    class Lock
    {
    public:
        Lock(Mutex* pSection){
            m_pSection = &pSection->m_crit;
            EnterCriticalSection(m_pSection);
        }
        ~Lock(){
            LeaveCriticalSection(m_pSection);
        }
    private:
        LPCRITICAL_SECTION m_pSection;
    };
	
	class TimeLock
	{
	public:
		TimeLock(Mutex* pSection, int t) :m_pSection(0), bReady(false)
		{
			int count = t / 5;
			for (int i = 0; i < count; i++)
			{
				if (TryEnterCriticalSection(&pSection->m_crit))
				{
					bReady = true;
					break;
				}
			}
			if (bReady)
				m_pSection = &pSection->m_crit;
		}
		~TimeLock(){
			if (bReady)
				LeaveCriticalSection(m_pSection);
		}
		bool IsReady(){
			return bReady;
		}
	private:
		bool bReady;
		LPCRITICAL_SECTION m_pSection;
	};
}

#endif //_LOCK_H_
