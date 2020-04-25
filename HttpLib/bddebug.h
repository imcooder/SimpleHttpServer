 
#ifndef BDDEBUG_H
#define	BDDEBUG_H

#include <crtdbg.h>
#include <assert.h>
#include <malloc.h>
#include <tchar.h>
#include <Strsafe.h>
#include <wctype.h>
#include <synchapi.h>
#ifndef VERIFY
#ifdef _DEBUG
#define VERIFY(f)          assert((f))//_ASSERT((f))
#else
#define VERIFY(f)          ((void)(f))
#endif    
#endif
#define verify   VERIFY
#ifndef ASSERT
#define ASSERT							assert
#endif

#ifndef ASSERT
#define ASSERT assert
#endif


#ifndef _abs
#define _abs(a) (((a)>0)?(a):(-(a)))
#endif

#ifndef _max
#define _max(a,b) ((a)>(b)?(a):(b))
#endif

#ifndef  _min
#define _min(a,b) ((a)<(b)?(a):(b))
#endif

#define _swap(a,b) {(a)^=(b);(b)^=(a);(a)^=(b);}

#define MAKELOW(c) (('A' <= (c) && (c) <= 'Z') ? 'a' + (c) - 'A' : (c))

#ifndef _countof
#define _countof(_Array) (sizeof(_Array) / sizeof(*_Array))
#endif

#ifndef ZeroMemory 
#define ZeroMemory(pVoid, nLen) memset((pVoid), 0, (nLen))
#endif

#define MSG_LEVEL_DEBUG					(0x10)
#define MSG_LEVEL_INFO					(0x20)
#define MSG_LEVEL_WARNING				(0x30)
#define MSG_LEVEL_ERROR					(0x40)
#define MSG_LEVEL_CRITICAL			(0x50)


#ifndef FINAL_RELEASE
#define ENABLE_LOG_OUTPUT
#endif

#ifdef ENABLE_LOG_OUTPUT

#ifndef DEBUGMSG
#if defined(DEBUG) || defined(_DEBUG)
#define DEBUGMSG		CDBG::GetInstance().Trace
#else 
#define DEBUGMSG		__noop
#endif
#endif
#ifndef TRACE
#define TRACE(...)	DEBUGMSG(MSG_LEVEL_DEBUG, __VA_ARGS__)
#endif
#define RETAILMSG		CDBG::GetInstance().Trace
#else		//ENABLE_LOG_OUTPUT

#define TRACE				__noop
#define DEBUGMSG		__noop
#define RETAILMSG __noop

#endif	//ENABLE_LOG_OUTPUT

#ifndef BD_AUTO_RELEASE_CONCAT_INNER
#define BD_AUTO_RELEASE_CONCAT_INNER(a,b) a##b
#endif
#ifndef BD_AUTO_RELEASE_CONCAT
#define BD_AUTO_RELEASE_CONCAT(a,b) BD_AUTO_RELEASE_CONCAT_INNER(a,b)
#endif
#ifndef BDAR_LINE_NAME
#define BDAR_LINE_NAME(prefix) BD_AUTO_RELEASE_CONCAT(prefix,__LINE__)
#endif
#ifndef ON_LEAVE
#define ON_LEAVE(statement) \
struct BDAR_LINE_NAME(ols_) { \
	~BDAR_LINE_NAME(ols_)() { statement; } \
} BDAR_LINE_NAME(olv_);
#endif
#ifndef ON_LEAVE_1
#define ON_LEAVE_1(statement, type, var) \
struct BDAR_LINE_NAME(ols_) { \
	type var; \
	BDAR_LINE_NAME(ols_)(type v): var(v) {} \
	~BDAR_LINE_NAME(ols_)() { statement; } \
} BDAR_LINE_NAME(olv_)(var);
#endif
#ifndef ON_LEAVE_2
#define ON_LEAVE_2(statement, type1, var1, type2, var2) \
struct BDAR_LINE_NAME(ols_) { \
	type1 var1; type2 var2; \
	BDAR_LINE_NAME(ols_)(type1 v1, type2 v2): var1(v1), var2(v2) {} \
	~BDAR_LINE_NAME(ols_)() { statement; } \
} BDAR_LINE_NAME(olv_)(var1, var2);
#endif
#ifndef ON_LEAVE_3
#define ON_LEAVE_3(statement, type1, var1, type2, var2, type3, var3) \
struct BDAR_LINE_NAME(ols_) { \
	type1 var1; type2 var2; type3 var3; \
	BDAR_LINE_NAME(ols_)(type1 v1, type2 v2, type3 v3): var1(v1), var2(v2), var3(v3) {} \
	~BDAR_LINE_NAME(ols_)() { statement; } \
} BDAR_LINE_NAME(olv_)(var1, var2, var3);
#endif
#ifndef ON_LEAVE_4
#define ON_LEAVE_4(statement, type1, var1, type2, var2, type3, var3, type4, var4) \
struct BDAR_LINE_NAME(ols_) { \
	type1 var1; type2 var2; type3 var3; type4 var4; \
	BDAR_LINE_NAME(ols_)(type1 v1, type2 v2, type3 v3, type4 v4): var1(v1), var2(v2), var3(v3), var4(v4) {} \
	~BDAR_LINE_NAME(ols_)() { statement; } \
} BDAR_LINE_NAME(olv_)(var1, var2, var3, var4);
#endif

#ifndef MULTI_THREAD_GUARD
#define MULTI_THREAD_GUARD(cs) \
	EnterCriticalSection(&cs); \
	ON_LEAVE_1(LeaveCriticalSection(&cs), CRITICAL_SECTION&, cs)
#endif
///////////////////////////////////////////////////////////////////////////////
struct XLogTag
{
    explicit XLogTag(const wchar_t* t) : tag(t) {}
    const wchar_t* tag;
};

class CDBG
{	
public:
	~CDBG();
private:
	CDBG();	
public:	
	void Output(int uLevel, const wchar_t* psz);
    static bool valueToQuotedString(const wchar_t* value, wchar_t* pszOut, int nlen);
public:
	static CDBG& GetInstance();
	void SetOutputFilter(int uFilter);
	void SetOutputJsonType(bool bJson);		
	void Trace(int dwLevel, const wchar_t* pwhFormat, ... );
	void Trace(int dwLevel, XLogTag tag, const wchar_t* pwhFormat, ...);
private:
	CRITICAL_SECTION m_CS;
	int m_uFilter;
	bool m_bJsonData;
	unsigned int m_dwPID;
	wchar_t m_szPName[512];
	bool m_isAvailable;
};



#define DISALLOW_COPY_AND_ASSIGN(TypeName)    \
	TypeName(const TypeName&);                    \
	void operator=(const TypeName&)

#ifndef DO_MSG4
#define DO_MSG4(message, fn)    \
		case (message): return fn((hWnd), (message), (wParam), (lParam))
#endif
#ifndef DO_MSG2
#define DO_MSG2(message, fn)    \
		case (message): return fn((wParam), (lParam))
#endif


#define NOCOPY(T)            T(const T&); T& operator=(const T&)



#ifndef WIDESTRING
#define WIDESTRING2(str) L##str
#define WIDESTRING(str) WIDESTRING2(str)
#endif

#define DEBUG_BREAK(expr) 


#define BD_REPORT_ERROR_2(expr) \
{\
	DEBUG_BREAK(WIDESTRING(expr));\
HRESULT _hr_ = HRESULT_FROM_WIN32(::GetLastError());\
	CDBG::GetInstance().Trace(MSG_LEVEL_ERROR, L"ASSERT(%s) LastError=0X%X @ %S(%d)", L"1", _hr_, __FILE__, __LINE__);	\
}

#define BD_VERIFY_BASE_2(expr, exprstr, statement) \
{ \
	if (!(expr)) { BD_REPORT_ERROR_2(exprstr);statement; } \
}

#define BD_VERIFY(expr, statement) \
	BD_VERIFY_BASE_2(expr, #expr, statement)

#ifndef DISABLE_XLOG

#define XLogDebug(...)  CDBG::GetInstance().Trace(MSG_LEVEL_DEBUG, __VA_ARGS__);
#define XLogInfo(...)  CDBG::GetInstance().Trace(MSG_LEVEL_INFO, __VA_ARGS__);
#define XLogWarning(...)  CDBG::GetInstance().Trace(MSG_LEVEL_WARNING, __VA_ARGS__);
#define XLogError(...)  CDBG::GetInstance().Trace(MSG_LEVEL_ERROR, __VA_ARGS__);


#else
#define XLogDebug         __noop
#define XLogInfo          __noop
#define XLogWarning       __noop
#define XLogError         __noop
#define XLogCritical      __noop
#endif

#define NET_DBG(...)  XLogDebug( __VA_ARGS__);
#define NET_LOG(...)  XLogInfo( __VA_ARGS__);
#define NET_ERR(...)  XLogError(__VA_ARGS__);
#define NET_WAR(...)  XLogWarning(__VA_ARGS__);
#define NET_CRT(...)  XLogCritical(__VA_ARGS__);

#define XTAG(str) XLogTag(str)
#define XTAGDEFAULT XLogTag(L"")

#if defined(DISABLE_XLOG_FUNCTION) || defined(DISABLE_XLOG)
#define XLOG_FUNCTION
#define XLOG_FUNCTION_WITH_PARAM1(fmt1, arg1)
#define XLOG_FUNCTION_WITH_PARAM2(fmt1, arg1, fmt2, arg2)
#define XLOG_FUNCTION_WITH_PARAM3(fmt1, arg1, fmt2, arg2, fmt3, arg3)
#define XLOG_FUNCTION_WITH_PARAM4(fmt1, arg1, fmt2, arg2, fmt3, arg3, fmt4, arg4)
#define XLOG_FUNCTION_WITH_PARAM5(fmt1, arg1, fmt2, arg2, fmt3, arg3, fmt4, arg4, fmt5, arg5)
#define XLOG_FUNCTION_WITH_PARAM6(fmt1, arg1, fmt2, arg2, fmt3, arg3, fmt4, arg4, fmt5, arg5, fmt6, arg6)
#else
#define XLOG_FUNCTION \
	XLogDebug(XTAG(L"func"), L"FUNCTION: %s", WIDESTRING(__FUNCTION__));

#define XLOG_FUNCTION_WITH_PARAM1(fmt1, arg1) \
	XLogDebug(XTAG(L"func"), L"FUNCTION: %s(" WIDESTRING(#arg1) L"=" fmt1 L")", WIDESTRING(__FUNCTION__), arg1);


#define XLOG_FUNCTION_WITH_PARAM2(fmt1, arg1, fmt2, arg2) \
	XLogDebug(XTAG(L"func"), L"FUNCTION: %s( " \
	WIDESTRING(#arg1) L"=" fmt1 \
	L" , " \
	WIDESTRING(#arg2) L"=" fmt2 \
	L" )", \
	WIDESTRING(__FUNCTION__), arg1, arg2);


#define XLOG_FUNCTION_WITH_PARAM3(fmt1, arg1, fmt2, arg2, fmt3, arg3) \
	XLogDebug(XTAG(L"func"), L"FUNCTION: %s( " \
	WIDESTRING(#arg1) L"=" fmt1 \
	L" , " \
	WIDESTRING(#arg2) L"=" fmt2 \
	L" , " \
	WIDESTRING(#arg3) L"=" fmt3 \
	L" )", \
	WIDESTRING(__FUNCTION__), arg1, arg2, arg3);


#define XLOG_FUNCTION_WITH_PARAM4(fmt1, arg1, fmt2, arg2, fmt3, arg3, fmt4, arg4) \
	XLogDebug(XTAG(L"func"), L"FUNCTION: %s( " \
	WIDESTRING(#arg1) L"=" fmt1 \
	L" , " \
	WIDESTRING(#arg2) L"=" fmt2 \
	L" , " \
	WIDESTRING(#arg3) L"=" fmt3 \
	L" , " \
	WIDESTRING(#arg4) L"=" fmt4 \
	L" )", \
	WIDESTRING(__FUNCTION__), arg1, arg2, arg3, arg4);


#define XLOG_FUNCTION_WITH_PARAM5(fmt1, arg1, fmt2, arg2, fmt3, arg3, fmt4, arg4, fmt5, arg5) \
	XLogDebug(XTAG(L"func"), L"FUNCTION: %s( " \
	WIDESTRING(#arg1) L"=" fmt1 \
	L" , " \
	WIDESTRING(#arg2) L"=" fmt2 \
	L" , " \
	WIDESTRING(#arg3) L"=" fmt3 \
	L" , " \
	WIDESTRING(#arg4) L"=" fmt4 \
	L" , " \
	WIDESTRING(#arg5) L"=" fmt5 \
	L" )", \
	WIDESTRING(__FUNCTION__), arg1, arg2, arg3, arg4, arg5); 	

#define XLOG_FUNCTION_WITH_PARAM6(fmt1, arg1, fmt2, arg2, fmt3, arg3, fmt4, arg4, fmt5, arg5, fmt6, arg6) \
	XLogDebug(XTAG(L"func"), L"FUNCTION: %s( " \
	WIDESTRING(#arg1) L"=" fmt1 \
	L" , " \
	WIDESTRING(#arg2) L"=" fmt2 \
	L" , " \
	WIDESTRING(#arg3) L"=" fmt3 \
	L" , " \
	WIDESTRING(#arg4) L"=" fmt4 \
	L" , " \
	WIDESTRING(#arg5) L"=" fmt5 \
	L" , " \
	WIDESTRING(#arg6) L"=" fmt6 \
	L" )", \
	WIDESTRING(__FUNCTION__), arg1, arg2, arg3, arg4, arg5, arg6); 

#define XLOG_FUNCTION_WITH_PARAM7(fmt1, arg1, fmt2, arg2, fmt3, arg3, fmt4, arg4, fmt5, arg5, fmt6, arg6, fmt7, arg7) \
	XLogDebug(XTAG(L"func"), L"FUNCTION: %s( " \
	WIDESTRING(#arg1) L"=" fmt1 \
	L" , " \
	WIDESTRING(#arg2) L"=" fmt2 \
	L" , " \
	WIDESTRING(#arg3) L"=" fmt3 \
	L" , " \
	WIDESTRING(#arg4) L"=" fmt4 \
	L" , " \
	WIDESTRING(#arg5) L"=" fmt5 \
	L" , " \
	WIDESTRING(#arg6) L"=" fmt6 \
	L" , " \
	WIDESTRING(#arg7) L"=" fmt7 \
	L" )", \
	WIDESTRING(__FUNCTION__), arg1, arg2, arg3, arg4, arg5, arg6, arg7); 

#endif


#endif//file

