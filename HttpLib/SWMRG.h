
#pragma once

class CSWMRG
{
public:
	class CAutoReadLock
	{
	public:
		CAutoReadLock(CSWMRG& swmrg) : m_swmrg(swmrg)
		{
			m_swmrg.WaitToRead();
		}
		~CAutoReadLock()
		{
			m_swmrg.Done();
		}
	private:
		CSWMRG& m_swmrg;
	};

	class CAutoWriteLock
	{
	public:
		CAutoWriteLock(CSWMRG& swmrg) : m_swmrg(swmrg)
		{
			m_swmrg.WaitToWrite();
		}
		~CAutoWriteLock()
		{
			m_swmrg.Done();
		}
	private:
		CSWMRG& m_swmrg;
	};
public:
	CSWMRG(void);
	~CSWMRG(void);
	void WaitToRead();
	void WaitToWrite();
	void Done();

private:
	CRITICAL_SECTION m_cs;
	HANDLE m_hsemReaders;
	HANDLE m_hsemWriters;
	int m_nWaitingReaders;
	int m_nWaitingWriters;
	int m_nActive;

};