#ifndef _TEMPLATE_THREAD_H_
#define _TEMPLATE_THREAD_H_

#include <process.h>

#define DEFAULT_STACK_SIZE 0

template<typename T>
class CThread
{
public:
	typedef int (T::*pThreadFunc)(void* pParam);
	typedef unsigned int  ThreadID_t;
public:
	CThread()
	{
		m_pOwner = NULL;
		m_pFunc = NULL;
		m_pParam = NULL;
		m_bRunning = false;
		m_stackSize = DEFAULT_STACK_SIZE;
		m_hThread = NULL;
		m_hTerminateEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	}

	CThread(T* pOwner, pThreadFunc ptFunc, void *param = NULL)
	{
		SetOwner(pOwner);
		m_pFunc = ptFunc;
		m_pParam = param;
		m_bRunning = false;
		m_stackSize = DEFAULT_STACK_SIZE;
		m_hThread = NULL;
		m_hTerminateEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	}

	~CThread()
	{
		if(m_hTerminateEvent != NULL)
		{
			CloseHandle(m_hTerminateEvent);
			m_hTerminateEvent = NULL;
		}
		if(m_hThread != NULL)
		{
			CloseHandle(m_hThread);
			m_hThread = NULL;
		}
	}

	bool AssignTask(T* pOwner, pThreadFunc ptFunc, void *param = NULL);

	bool Start();
	void Stop(int dwTimeouts = INFINITE);
	bool Resume();
	bool Suspend();
	void Execute();

	static unsigned int __stdcall ThreadProcess(void *lpParam);

	ThreadID_t GetThreadID() const
	{
		return m_ThreadId;
	}

	void SetPriority(int priority)
	{
		SetThreadPriority(m_hThread, priority);
	}

	HANDLE &GetTerminateEvent() const
	{
		return m_hTerminateEvent;
	}

private:
	void SetOwner(T* pOwner)
	{
		::InterlockedExchangePointer(reinterpret_cast<void**>(&m_pOwner), pOwner);
	}

private:
	T* m_pOwner;
	pThreadFunc m_pFunc;
	void *m_pParam;

	HANDLE m_hThread;
	HANDLE	m_hTerminateEvent; 

	ThreadID_t  m_ThreadId;
	int m_stackSize;
	bool m_bRunning;
};

template<typename T>
bool CThread<T>::Start()
{
	if(m_bRunning)
		return true;

	if(m_hThread != NULL)
	{
		CloseHandle(m_hThread);
		m_hThread = NULL;
	}

	m_hThread = (HANDLE)_beginthreadex(NULL, m_stackSize, ThreadProcess, this, 0, &m_ThreadId);
	if(m_hThread == NULL)
		return false;

	m_bRunning = true;
	return true;
}

template<typename T>
void CThread<T>::Stop(int dwTimeouts)
{
	m_bRunning = false;
	if(m_hThread != NULL)
	{
		SetEvent(m_hTerminateEvent);
		if ( WaitForSingleObject(m_hThread, dwTimeouts) == WAIT_TIMEOUT ) 
		{
			TerminateThread(m_hThread, 1);
		}
		CloseHandle(m_hThread);
		m_hThread = NULL;
	}
}

template<typename T>
bool CThread<T>::Resume()
{
	if(ResumeThread(m_hThread) == -1)
		return false;
	return true;
}

template<typename T>
bool CThread<T>::Suspend()
{
	if(SuspendThread(m_hThread) == -1)
		return false;
	return true;
}

template<typename T>
void CThread<T>::Execute()
{
	if(m_pOwner != NULL && m_pFunc != NULL)
	{
		(m_pOwner->*(m_pFunc))(m_pParam);
	}
}

template<typename T>
bool CThread<T>::AssignTask(T* pOwner, pThreadFunc ptFunc, void* param/* = NULL*/)
{
	if(pOwner == NULL || ptFunc == NULL)
		return false;

	SetOwner(pOwner);
	m_pFunc = ptFunc;
	m_pParam = param;

	return true;
}

template<typename T>
unsigned int __stdcall CThread<T>::ThreadProcess(void *lpParam)
{
	CThread *cls = (CThread *)lpParam;

	if (cls != NULL)
	{
		if(cls->m_bRunning)
		{
			cls->Execute();
		}
	}

	return 0;
}

#endif // _TEMPLATE_THREAD_H_
