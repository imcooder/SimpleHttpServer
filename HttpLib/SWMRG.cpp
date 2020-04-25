
#include "pch.h"
#include "SWMRG.h"

CSWMRG::CSWMRG(void)
: m_hsemReaders(NULL)
, m_hsemWriters(NULL)
{
	m_nWaitingReaders = m_nWaitingWriters = m_nActive = 0;
	m_hsemReaders = ::CreateSemaphore(NULL, 0, MAXLONG, NULL);
	m_hsemWriters = ::CreateSemaphore(NULL, 0, MAXLONG, NULL);
	::InitializeCriticalSection(&m_cs);
}

CSWMRG::~CSWMRG(void)
{
	m_nWaitingReaders = m_nWaitingWriters = m_nActive = 0;
	DeleteCriticalSection(&m_cs);
	CloseHandle(m_hsemReaders);
	m_hsemReaders = NULL;
	CloseHandle(m_hsemWriters);
	m_hsemWriters = NULL;
}

void CSWMRG::WaitToRead()
{
	EnterCriticalSection(&m_cs);
	bool fResourceWritePending = (m_nWaitingWriters || (m_nActive < 0));
	if (fResourceWritePending)
	{
		m_nWaitingReaders++;
	}
	else
	{
		m_nActive++;
	}
	LeaveCriticalSection(&m_cs);
	if (fResourceWritePending)
	{
		::WaitForSingleObject(m_hsemReaders, INFINITE);
	}
}

void CSWMRG::WaitToWrite()
{
	EnterCriticalSection(&m_cs);
	bool fResourceOwned = (m_nActive != 0);
	if (fResourceOwned)
	{
		m_nWaitingWriters++;
	}
	else
	{
		m_nActive = -1;
	}
	LeaveCriticalSection(&m_cs);
	if (fResourceOwned)
	{
		::WaitForSingleObject(m_hsemWriters, INFINITE);
	}
}

void CSWMRG::Done()
{
	EnterCriticalSection(&m_cs);
	if (m_nActive > 0)
	{
		m_nActive--;
	}
	else
	{
		m_nActive++;
	}
	HANDLE hsem = NULL;
	LONG lCount = 1;
	if (m_nActive == 0)
	{
		if (m_nWaitingWriters > 0)
		{
			m_nActive = -1;
			m_nWaitingWriters--;
			hsem = m_hsemWriters;
		}
		else if (m_nWaitingReaders > 0)
		{
			m_nActive = m_nWaitingReaders;
			m_nWaitingReaders = 0;
			hsem = m_hsemReaders;
			lCount = m_nActive;
		}
	}
	::LeaveCriticalSection(&m_cs);
	if (hsem != NULL)
	{
		::ReleaseSemaphore(hsem, lCount, NULL);
	}
}