
#include "pch.h"
#include "bddebug.h"
static bool isControlCharacter( TCHAR ch )
{
    return ch > 0 && ch <= 0x1F;
}
static bool containsControlCharacter( const TCHAR* str )
{
    while ( *str ) 
    {
        if ( isControlCharacter( *(str++) ) )
            return true;
    }
    return false;
}
bool CDBG::valueToQuotedString( const wchar_t *value, wchar_t* pszOut, int nlen )
{
    // Not sure how to handle unicode...
    if (pszOut == NULL || nlen < 10)
    {
        return false;
    }
    if (_tcspbrk(value, _T("\"\\\b\f\n\r\t")) == NULL && !containsControlCharacter( value ))
    {			
        StringCchCopy(pszOut, nlen, _T("\""));
        StringCchCat(pszOut, nlen - 2, value);
        StringCchCat(pszOut, nlen, _T("\""));
        return true;
    }
    StringCchCat(pszOut, nlen, _T("\""));
    for (const wchar_t* c = value; *c != 0; ++c)
    {
        wchar_t sz[2] = { *c, 0};
        switch(*c)
        {
        case '\"':
            StringCchCat(pszOut, nlen - 2, _T("\\\""));
            break;
        case '\\':				
            StringCchCat(pszOut, nlen - 2, _T("\\\\"));
            break;
        case '\b':				
            StringCchCat(pszOut, nlen - 2, _T("\\b"));
            break;
        case '\f':				
            StringCchCat(pszOut, nlen - 2, _T("\\f"));
            break;
        case '\n':			
            StringCchCat(pszOut, nlen - 2, _T("\\n"));
            break;
        case '\r':
            StringCchCat(pszOut, nlen - 2, _T("\\r"));				
            break;
        case '\t':
            StringCchCat(pszOut, nlen - 2, _T("\\t"));
            break;				
        default:
            if ( isControlCharacter( *c ) )
            {
                TCHAR szTmp[16] = {0};					
                StringCchPrintf(szTmp, _countof(szTmp), _T("\\u%04X"), sz);
                StringCchCat(pszOut, nlen - 2, szTmp);
            }
            else
            {
                StringCchCat(pszOut, nlen - 2, sz);
            }
            break;
        }
    }		
    StringCchCat(pszOut, nlen, _T("\""));
    return true;
}
CDBG::~CDBG()
{
    ::DeleteCriticalSection(&m_CS);
    m_isAvailable = false;
}

CDBG::CDBG() : m_uFilter((ULONG_PTR)-1)
    , m_bJsonData(true)
    , m_isAvailable(true)
{
    ::InitializeCriticalSection(&m_CS);
    wchar_t szProcess[MAX_PATH] = {0};
    GetModuleFileName(NULL, szProcess, _countof(szProcess));
    LPCTSTR pszName = _tcsrchr(szProcess, _T('\\'));
    if (!pszName)
    {
        pszName = szProcess;
    }
    else
    {
        pszName ++;
    }
    StringCchCopy(m_szPName, _countof(m_szPName), pszName);
    m_dwPID = GetCurrentProcessId();
}



void CDBG::Output( int uLevel, const wchar_t* psz )
{
    if (psz == NULL || *psz == 0)
    {
        return;
    }
    if (!m_bJsonData)
    {
        OutputDebugString(psz);
        return;
    }
    wchar_t szContext2[1024 * 2] = {0};
    valueToQuotedString(psz, szContext2, _countof(szContext2));		
    SYSTEMTIME time;
    GetLocalTime(&time);

    wchar_t szFormat[1024 * 4] = {0};
    StringCchPrintf(szFormat, _countof(szFormat),
        _T("{\n\
           \"class\" : %d,\n\
           \"content\" : %s,\n\
           \"pid\" : %d,\n\
           \"pname\" : \"%s\",\n\
           \"tid\" : %d,\n\
           \"time\" : \"%d-%d-%d %d:%d:%d.%d\"\n\
           }"), 
           uLevel, szContext2, m_dwPID, m_szPName, GetCurrentThreadId(), time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond, time.wMilliseconds);	
    OutputDebugString(szFormat);
}

CDBG& CDBG::GetInstance()
{
    static CDBG dbg;
    return dbg;
}

void CDBG::SetOutputFilter( int uFilter )
{
    if (!m_isAvailable)
    {
        return;
    }
    CRITICAL_SECTION* pCS = &m_CS;
    EnterCriticalSection(pCS); \
        ON_LEAVE_1(LeaveCriticalSection(pCS), CRITICAL_SECTION*, pCS)		
        m_uFilter = static_cast<ULONG>(uFilter);
    return;
}

void CDBG::SetOutputJsonType( bool bJson )
{
    if (!m_isAvailable)
    {
        return;
    }
    CRITICAL_SECTION* pCS = &m_CS;
    EnterCriticalSection(pCS); \
        ON_LEAVE_1(LeaveCriticalSection(pCS), CRITICAL_SECTION*, pCS)		
        m_bJsonData = bJson;
    return;
}

void CDBG::Trace( int dwLevel, const wchar_t* pwhFormat, ... )
{
    if (!m_isAvailable)
    {
        return;
    }
    CRITICAL_SECTION* pCS = &m_CS;
    EnterCriticalSection(pCS);
    ON_LEAVE_1(LeaveCriticalSection(pCS), CRITICAL_SECTION*, pCS);
    if (!(dwLevel & m_uFilter))
    {
        return;
    }
    wchar_t szBuffer[1024 * 1] = {0};
    va_list argList;
    va_start(argList, pwhFormat);			
    StringCchVPrintf(szBuffer, _countof(szBuffer) - 1, pwhFormat, argList); 		
    va_end(argList);		
    Output(dwLevel, szBuffer);
    return;
}
void CDBG::Trace(int dwLevel, XLogTag tag, const wchar_t* pwhFormat, ...)
{
    if (!m_isAvailable)
    {
        return;
    }
    CRITICAL_SECTION* pCS = &m_CS;
    EnterCriticalSection(pCS);
    ON_LEAVE_1(LeaveCriticalSection(pCS), CRITICAL_SECTION*, pCS);
    if (!(dwLevel & m_uFilter))
    {
        return;
    }
    wchar_t szBuffer[1024 * 1] = { 0 };
    va_list argList;
    va_start(argList, pwhFormat);
    StringCchVPrintf(szBuffer, _countof(szBuffer) - 1, pwhFormat, argList);
    va_end(argList);
    Output(dwLevel, szBuffer);
    return;
}
