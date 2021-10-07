//
// cthread.h
//

#ifndef _INC_CTHREAD_H
#define _INC_CTHREAD_H
#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef _WIN32
#include <pthread.h>
#endif

#ifndef _AFXDLL	// Not sure how to make this MFC compatible. yet

#ifdef _WIN32
#define THREAD_ENTRY_RET void
#else	// else LINUX
#define THREAD_ENTRY_RET void *
#endif
typedef THREAD_ENTRY_RET ( _cdecl * PTHREAD_ENTRY_PROC )(void *);

class CThread	// basic multi tasking functionality.
{
protected:
	DWORD m_dwThreadID;	// unique thread id. ie. stack base pointer.
#ifdef _WIN32
	DWORD m_hThread;	// In windows there may be many handles to the same thread.
#else
	pthread_t m_hThread;
#endif
public:
	CThread()
	{
		m_dwThreadID = 0;
		m_hThread = 0;
	}
	~CThread()
	{
		TerminateThread( (DWORD) -1 );
	}
	static DWORD GetCurrentThreadId();
	void OnClose();
	void OnCreate()
	{
		// The id for the thread is diff from the handle.
		// There may be many handles but only one id !
		m_dwThreadID = GetCurrentThreadId();	
	}
	DWORD GetThreadID() const
	{
		return( m_dwThreadID );
	}
	bool IsActive() const
	{
		return( m_hThread ? true : false );
	}
	bool TerminateThread( DWORD dwExitCode );
	void CreateThread( PTHREAD_ENTRY_PROC pEntryProc, void * pArgs );
	void CreateThread( PTHREAD_ENTRY_PROC pEntryProc );
	void WaitForClose( int iSec );

	// Periodically called to check if this thread is stuck. if so, then do something about it !
	virtual void CheckStuckThread() 
	{
	}
};

class CThreadLockableObj
{
	// Base class for any data structure that may be locked for multi threaded access.
	friend class CThreadLockRef;
private:
#ifdef _WIN32
	CRITICAL_SECTION m_LockSection;	// RTL_CRITICAL_SECTION
#else
	pthread_mutex_t m_LockSection;
#endif

private:
	void DoThreadLock();
	void UnThreadLock();
public:
	CThreadLockableObj();
	~CThreadLockableObj();
	bool IsLocked();
};

class CThreadLockRef
{
	// Reference link to a CThreadLockableObj object.
	// An instance of a Spinlock on a data object.
	// Use this as an automatic (stack) defined var.
	//  throw will call destructors.
	// use this for adding and delete from lists is a must.
private:
	CThreadLockableObj* m_pLink;
public:
	CThreadLockableObj* GetRef() const
	{
		return(m_pLink);
	}
	operator CThreadLockableObj*() const
    {
		return(m_pLink);
    }
	bool IsRefValid() const
	{
		return( m_pLink != NULL );
	}

	void ReleaseRef();
	void SetRef( CThreadLockableObj* pLink );

	CThreadLockRef& operator =( CThreadLockableObj * pLink )
    {
		SetRef(pLink);
		return(*this);
    }
	CThreadLockRef::CThreadLockRef( CThreadLockableObj * pLockThis )
	{
		m_pLink = NULL;
		SetRef( pLockThis );
	}
	CThreadLockRef()
	{
		m_pLink = NULL;
	}
	~CThreadLockRef()
	{
		ReleaseRef();
	}
};

#endif	// _AFXDLL

#endif	// _INC_CTHREAD_H
