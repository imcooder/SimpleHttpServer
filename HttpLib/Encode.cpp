#include "pch.h"
#include "Encode.h"
#include <openssl/md5.h>
#include <atlutil.h>
#define TOHEX(x) ((x) > 9 ? (x) - 10 + 'A' : (x) + '0')
#define FROMHEX(x) ('0' <= (x) && (x) <= '9' ? (x) - '0' : (x) - 'A' + 10)

#pragma comment(lib, "libeay32")

static const char hex_chars[] = "0123456789abcdef";
namespace Util
{
    namespace Code
    {
        std::wstring Md5BinW(const unsigned char*pbData, size_t size)
        {
            return Md5::Encode2W(pbData, size);
        }
        std::string Md5Bin(const unsigned char*pbData, size_t size)
        {
            return Md5::Encode(pbData, size);
        }       

    };
    namespace Md5
    {
        std::wstring Encode2W(const unsigned char*pbData, size_t size)
        {
            BD_VERIFY(pbData && size, return L"");
            MD5_CTX md5;
            MD5_Init(&md5);
            MD5_Update(&md5, pbData, size);
            unsigned char md[64] = { 0 };
            MD5_Final(md, &md5);
            wchar_t szMd5[33] = { 0 };
            for (int i = 0; i < 16; i++)
            {
                szMd5[i * 2] = hex_chars[(md[i] >> 4) & 0x0F];
                szMd5[i * 2 + 1] = hex_chars[md[i] & 0x0F];
            }
            szMd5[32] = L'\0';
            return szMd5;
        }
        std::string Encode(const unsigned char*pbData, size_t size)
        {
            BD_VERIFY(pbData && size, return "");
            MD5_CTX md5;
            MD5_Init(&md5);
            MD5_Update(&md5, pbData, size);
            unsigned char md[64] = { 0 };
            MD5_Final(md, &md5);
            char szMd5[33] = { 0 };
            for (int i = 0; i < 16; i++)
            {
                szMd5[i * 2] = hex_chars[(md[i] >> 4) & 0x0F];
                szMd5[i * 2 + 1] = hex_chars[md[i] & 0x0F];
            }
            szMd5[32] = '\0';
            return szMd5;
        }

        std::string Encode(const std::string&strValue)
        {
            return Encode((const unsigned char*)strValue.c_str(), strValue.length());
        }

    };
    namespace Hex
    {
        std::string Encode(const LPBYTE data, DWORD dlen)
        {
            std::string ret;
            if (data == NULL || dlen == 0)
            {
                return ret;
            }
            ret.reserve(dlen * 2 + 1);
            UINT j = 0;
            LPCSTR hexval = "0123456789abcdef";
            for (; j < dlen; j++)
            {
                ret.append(&hexval[((data[j] >> 4) & 0x0F)], 1);
                ret.append(&hexval[(data[j]) & 0x0F], 1);
            }
            return ret;
        }

        std::wstring EncodeW(const LPBYTE data, DWORD dlen)
        {
            std::wstring ret;
            if (data == NULL || dlen == 0)
            {
                return ret;
            }
            ret.reserve(dlen * 2 + 1);
            LPCWSTR hexval = L"0123456789abcdef";
            for (DWORD j = 0; j < dlen; j++)
            {
                ret.append(&hexval[((hexval[j] >> 4) & 0x0F)], 1);
                ret.append(&hexval[(hexval[j]) & 0x0F], 1);
            }
            return ret;
        }
        std::string Decode(const char*psz, size_t uLen)
        {
            if (!psz || !uLen)
            {
                return "";
            }
            std::string out;
            for (size_t i = 0; i < uLen / 2; i ++)
            {
                BYTE b = A2B(psz[i * 2]);
                BYTE b1 = A2B(psz[i * 2 + 1]);
                if (b > 0xf || b1 > 0xf)
                {
                    return "";
                }
                BYTE res = (b << 4) | b1;
                out.append((char*)&res, 1);
            }
            return out;
        }

        std::string Decode(const std::string&strInput)
        {
            return Decode(strInput.c_str(), strInput.length());
        }

        std::string DecodeW(const WCHAR*psz, size_t uLen)
        {
            if (!psz || !uLen)
            {
                return "";
            }
            std::string out;
            for (size_t i = 0; i < uLen / 2; i ++)
            {
                BYTE res = (psz[i * 2] << 4) | psz[i * 2 + 1];
                out.append((char*)&res, 1);
            }
            return out;
        }
    }
    namespace URL
    {
        static BOOL URLIsChar(const char ch)
        {
            if ('A' <= ch && ch <= 'Z')
                return TRUE;
            else if ('a' <= ch && ch <= 'z')
                return TRUE;
            else if ('0' <= ch && ch <= '9')
                return TRUE;
            else
                return FALSE;
        }
        static BOOL URLIsDigit(const char ch)
        {
            return ('0' <= ch && ch <= '9');
        }

        std::wstring EncodeW(const wchar_t*pszInput, size_t uLen, DWORD codepage)
        {
            CString str = Encode(pszInput, uLen, codepage).c_str();
            return (LPCWSTR)str;
        }
        std::wstring EncodeW2W(const wchar_t*pszInput, size_t uLen, DWORD codepage /*= CP_UTF8*/)
        {
            CString str = Encode(pszInput, uLen, codepage).c_str();
            return (LPCWSTR)str;
        }
        string Encode(const char* lpszVal, size_t lenVal)
        {
            string strRet;
            if (NULL == lpszVal)
            {
                strRet = "";
                return strRet;
            }
            if (0 == lenVal || 0 == lpszVal[0])
            {
                strRet = "";
                return strRet;
            }
            _ASSERT(0 < (long)lenVal && lenVal < 0x0000FFFF);
            for (const char* pszLoop = lpszVal; 0 != *pszLoop && pszLoop < lpszVal + lenVal; ++pszLoop)
            {
                char ch = *pszLoop;
                if (URLIsChar(*pszLoop))
                {
                    strRet += *pszLoop;
                }
                else
                {
                    strRet += '%';
                    strRet += TOHEX((BYTE)ch >> 4);
                    strRet += TOHEX((BYTE)ch % 16);
                }
            }
            return strRet;
        }

        std::wstring Encode2W(const char* lpszVal, size_t lenVal)
        {
            wstring strRet;
            if (NULL == lpszVal)
            {
                strRet = L"";
                return strRet;
            }
            if (0 == lenVal || 0 == lpszVal[0])
            {
                strRet = L"";
                return strRet;
            }
            _ASSERT(0 < (long)lenVal && lenVal < 0x0000FFFF);
            for (const char* pszLoop = lpszVal; 0 != *pszLoop && pszLoop < lpszVal + lenVal; ++pszLoop)
            {
                char ch = *pszLoop;
                if (URLIsChar(*pszLoop))
                {
                    strRet += (WCHAR)*pszLoop;
                }
                else
                {
                    strRet += L'%';
                    strRet += (WCHAR)TOHEX((BYTE)ch >> 4);
                    strRet += (WCHAR)TOHEX((BYTE)ch % 16);
                }
            }
            return strRet;
        }
        string Encode(const wchar_t* pszInput, size_t uLen, DWORD codepage /*= CP_UTF8*/)
        {
            char szBuffer[2049] = { 0 };
            int _Dsize = WideCharToMultiByte(codepage, 0, pszInput, uLen, NULL, 0, NULL, NULL);
            if (_Dsize <= 0)
            {
                return "";
            }
            if (_Dsize < _countof(szBuffer) - 1)
            {
                WideCharToMultiByte(codepage, 0, pszInput, uLen, szBuffer, _countof(szBuffer) - 1, NULL, NULL);
                szBuffer[_Dsize] = 0;
                return Encode(szBuffer, _Dsize);
            }
            char *_Dest = (char*)_alloca(_Dsize + 1);
            if (!_Dest)
            {
                return "";
            }
            WideCharToMultiByte(codepage, 0, pszInput, uLen, _Dest, _Dsize, NULL, NULL);
            _Dest[_Dsize] = 0;
            return Encode(_Dest, _Dsize);
        }

        string Encode(const std::string&strValue)
        {
            return Encode(strValue.c_str(), strValue.length());
        }

        string Decode(const char* lpszVal, size_t lenVal)
        {
            std::string sOut;
            if (!lpszVal || !lenVal)
            {
                return "";

            }
            const char*pszTail = lpszVal + lenVal;
            while (lpszVal < pszTail)
            {
                BYTE ch = 0;
                if (*lpszVal == '%')
                {
                    if (lpszVal + 2 >= pszTail)
                    {
                        break;
                    }
                    ch = (FROMHEX(lpszVal[1]) << 4);
                    ch |= FROMHEX(lpszVal[2]);
                    lpszVal += 3;
                }
                else if (*lpszVal == '+')
                {
                    ch = ' ';
                    lpszVal ++;
                }
                else
                {
                    ch = *lpszVal;
                    lpszVal ++;
                }
                sOut += (char)ch;
            }
            return sOut;
        }
        string Decode2(const wchar_t* lpszVal, size_t lenVal)
        {
            std::string sOut;
            if (!lpszVal || !lenVal)
            {
                return "";

            }
            const wchar_t*pszTail = lpszVal + lenVal;
            while (lpszVal < pszTail)
            {
                BYTE ch = 0;
                if (*lpszVal == L'%')
                {
                    if (lpszVal + 2 >= pszTail)
                    {
                        break;
                    }
                    ch = (FROMHEX(lpszVal[1]) << 4);
                    ch |= FROMHEX(lpszVal[2]);
                    lpszVal += 3;
                }
                else if (*lpszVal == L'+')
                {
                    ch = ' ';
                    lpszVal ++;
                }
                else
                {
                    ch = *lpszVal;
                    lpszVal ++;
                }
                sOut += (char)ch;
            }
            return sOut;
        }
        wstring DecodeW(const char* lpszVal, size_t lenVal, DWORD codepage)
        {
            string str = Decode(lpszVal, lenVal);
            WCHAR szBuffer[2049] = { 0 };
            if (str.empty())
            {
                return L"";
            }
            int _Dsize = MultiByteToWideChar(codepage, 0, str.c_str(), str.length(), NULL, 0);
            if (_Dsize < _countof(szBuffer) - 1)
            {
                MultiByteToWideChar(codepage, 0, str.c_str(), str.length(), szBuffer, _countof(szBuffer) - 1);
                szBuffer[_Dsize] = 0;
                return szBuffer;
            }
            else
            {
                wchar_t *_Dest = (wchar_t*)_alloca((_Dsize + 1) * sizeof(wchar_t));
                if (!_Dest)
                {
                    return L"";
                }
                MultiByteToWideChar(codepage, 0, str.c_str(), str.length(), _Dest, _Dsize);
                _Dest[_Dsize] = 0;
                return _Dest;
            }
            return L"";
        }
        wstring Decode2W(const wchar_t* lpszVal, size_t lenVal, DWORD codepage /*= CP_UTF8*/)
        {
            string str = Decode2(lpszVal, lenVal);
            WCHAR szBuffer[2049] = { 0 };
            if (str.empty())
            {
                return L"";
            }
            int _Dsize = MultiByteToWideChar(codepage, 0, str.c_str(), str.length(), NULL, 0);
            if (_Dsize < _countof(szBuffer) - 1)
            {
                MultiByteToWideChar(codepage, 0, str.c_str(), str.length(), szBuffer, _countof(szBuffer) - 1);
                szBuffer[_Dsize] = 0;
                return szBuffer;
            }
            else
            {
                wchar_t *_Dest = (wchar_t*)_alloca((_Dsize + 1) * sizeof(wchar_t));
                if (!_Dest)
                {
                    return L"";
                }
                MultiByteToWideChar(codepage, 0, str.c_str(), str.length(), _Dest, _Dsize);
                _Dest[_Dsize] = 0;
                return _Dest;
            }
            return L"";
        }
        bool CParser::ArgExist(const wstring&strName)
        {
            return m_argParser.ArgExist(strName);
        }
        wstring CParser::GetArgString(const wchar_t* name, const wchar_t*def)
        {
            return m_argParser.GetArgString(name, def);
        }
        __int64 CParser::GetArgInt64(const wchar_t* name, __int64 defaut_value)
        {
            return m_argParser.GetArgInt64(name, defaut_value);
        }
        int CParser::GetArgInt(const wchar_t* name, int defaut_value)
        {
            return m_argParser.GetArgInt(name, defaut_value);
        }
        bool CParser::GetArgBool(const wchar_t* name, bool defaut_value)
        {
            return m_argParser.GetArgBool(name, defaut_value);
        }
        void CParser::CrackUrl(LPCWSTR pszURL)
        {
            BD_VERIFY(pszURL && *pszURL, return);
            InitFields();
            ATL::CUrl url;
            url.CrackUrl(pszURL);
            m_strPath = url.GetUrlPath();
            m_strScheme = url.GetSchemeName();
            const WCHAR* pszData = wcschr(pszURL, L'?');
            if (pszData)
            {
                m_strPath.clear();
                m_strPath.append(pszURL, pszData - pszURL);
                pszData ++;
                m_argParser.Parse(pszData);
            }
            else
            {
                m_strPath = pszURL;
            }
            return;
        }
        void CParser::InitFields()
        {
            m_strPath.clear();
            m_strScheme.clear();
            m_argParser.Reset();
        }

        std::wstring CParser::GetPath()
        {
            return m_strPath;
        }

        void Parse(const std::string &url
                   , std::string*pstrScheme
                   , std::string*pstrHost
                   , std::string*pstrPath
                   , int*pnPort
                   , std::string*pstrArg)
        {
            std::string url_(url);
            string port;
            string strScheme;
            string strHost;
            string strPath;
            string strArg;
            int nPort = 0;
            std::string::size_type pos = url.find("://");
            if (pos != std::string::npos)
            {
                strScheme = url_.substr(0, pos);
                url_ = url_.substr(pos + 3);
            }
            pos = url_.find("/");
            if (pos == std::string::npos)
            {
                strHost = url_;
                strPath = "/";
            }
            else
            {
                strHost = url_.substr(0, pos);
                strPath = url_.substr(pos);
            }
            {
                std::size_t port_pos = strHost.find_first_of(":");
                if (port_pos != string::npos && port_pos > 0)
                {
                    port = strHost.substr(port_pos + 1);
                    strHost.erase(port_pos);
                }
            }
            {
                std::size_t first_question_pos = strPath.find_first_of("?");
                if (first_question_pos != string::npos && first_question_pos > 0)
                {
                    strArg = strPath.substr(first_question_pos + 1);
                    strPath.erase(first_question_pos);
                }
            }
            if (strScheme.empty())
            {
                strScheme = "http";
            }
            if (port.empty())
            {
                if (!_stricmp(strScheme.c_str(), "https"))
                {
                    nPort = 443;
                }
                else if (!_stricmp(strScheme.c_str(), "ftp"))
                {
                    nPort = 21;
                }
                else
                {
                    nPort = 80;
                }
            }
            else
            {
                nPort = atoi(port.c_str());
            }
            if (pstrScheme)
            {
                *pstrScheme = strScheme;
            }
            if (pstrHost)
            {
                *pstrHost = strHost;
            }
            if (pstrPath)
            {
                *pstrPath = strPath;
            }
            if (pnPort)
            {
                *pnPort = nPort;
            }
            if (pstrArg)
            {
                *pstrArg = strArg;
            }
            return;
        }
        void ParseW(const std::wstring &url
                    , std::wstring*pstrScheme
                    , std::wstring*pstrHost
                    , std::wstring*pstrPath
                    , int*pnPort
                    , std::wstring*pstrArg)
        {
            std::wstring url_(url);
            std::wstring port;
            std::wstring strScheme;
            std::wstring strHost;
            std::wstring strPath;
            std::wstring strArg;
            int nPort = 0;
            std::wstring::size_type pos = url.find(L"://");
            if (pos != std::string::npos)
            {
                strScheme = url_.substr(0, pos);
                url_ = url_.substr(pos + 3);
            }
            pos = url_.find(L"/");
            if (pos == std::string::npos)
            {
                strHost = url_;
                strPath = L"/";
            }
            else
            {
                strHost = url_.substr(0, pos);
                strPath = url_.substr(pos);
            }
            {
                std::size_t port_pos = strHost.find_first_of(L":");
                if (port_pos != string::npos && port_pos > 0)
                {
                    port = strHost.substr(port_pos + 1);
                    strHost.erase(port_pos);
                }
            }
            {
                std::size_t first_question_pos = strPath.find_first_of(L"?");
                if (first_question_pos != string::npos && first_question_pos > 0)
                {
                    strArg = strPath.substr(first_question_pos + 1);
                    strPath.erase(first_question_pos);
                }
            }
            if (strScheme.empty())
            {
                strScheme = L"http";
            }
            if (port.empty())
            {
                if (!_wcsicmp(strScheme.c_str(), L"https"))
                {
                    nPort = 443;
                }
                else if (!_wcsicmp(strScheme.c_str(), L"ftp"))
                {
                    nPort = 21;
                }
                else
                {
                    nPort = 80;
                }
            }
            else
            {
                nPort = _wtoi(port.c_str());
            }
            if (pstrScheme)
            {
                *pstrScheme = strScheme;
            }
            if (pstrHost)
            {
                *pstrHost = strHost;
            }
            if (pstrPath)
            {
                *pstrPath = strPath;
            }
            if (pnPort)
            {
                *pnPort = nPort;
            }
            if (pstrArg)
            {
                *pstrArg = strArg;
            }
            return;
        }

        std::string CArgParserA::GetArgString(const std::string& name, const std::string&def)
        {
            auto it = mapArgs.find(name);
            if (it == mapArgs.end())
            {
                return def;
            }
            if (it->second.empty())
            {
                return def;
            }
            return it->second;
        }

        __int64 CArgParserA::GetArgInt64(const std::string& name, __int64 defaut_value)
        {
            auto it = mapArgs.find(name);
            if (it == mapArgs.end())
            {
                return defaut_value;
            }
            if (it->second.empty())
            {
                return defaut_value;
            }
            return _atoi64(it->second.c_str());
        }

        int CArgParserA::GetArgInt(const std::string& name, int defaut_value)
        {
            auto it = mapArgs.find(name);
            if (it == mapArgs.end())
            {
                return defaut_value;
            }
            if (it->second.empty())
            {
                return defaut_value;
            }
            return stoi(it->second.c_str());
        }

        bool CArgParserA::ArgExist(const std::string&strName)
        {
            auto it = mapArgs.find(strName);
            if (it == mapArgs.end())
            {
                return false;
            }
            return true;
        }

        bool CArgParserA::GetArgBool(const std::string& name, bool defaut_value)
        {
            auto it = mapArgs.find(name);
            if (it == mapArgs.end())
            {
                return defaut_value;
            }
            if (it->second.empty())
            {
                return defaut_value;
            }
            if (!_stricmp(it->second.c_str(), "false")
                || it->second == "0")
            {
                return false;
            }
            return true;
        }
        void CArgParserA::Parse(const std::string&strArg)
        {
            if (strArg.empty())
            {
                return;
            }
            const char* pszData = strArg.c_str();
            const char *pszArgStart = NULL;
            const char*pszArgEnd = NULL;
            const char *pszValueStart = NULL;
            const char*pszEnd = NULL;
            const char*pszNextArg = NULL;
            pszEnd = pszData + strlen(pszData);
            pszNextArg = pszData;
            while (pszNextArg < pszEnd)
            {
                pszArgStart = pszNextArg;
                pszNextArg = (const char*)memchr((const void*)pszArgStart, 
                                    '&',
                                    pszEnd - pszArgStart);
                if (!pszNextArg)
                {
                    pszNextArg = pszEnd;
                }
                else
                {
                    pszNextArg ++;
                }
                pszArgEnd = pszNextArg;
                while (pszArgStart < pszArgEnd && isspace(*pszArgStart))
                {
                    pszArgStart ++;
                }
                while ((pszArgEnd > pszArgStart) && (isspace(pszArgEnd[-1]) || pszArgEnd[-1] == L'&'))
                {
                    pszArgEnd --;
                }
                if (pszArgStart >= pszArgEnd)
                {
                    continue;
                }
                int nLen = pszArgEnd - pszArgStart;
                if ((pszValueStart = (const char*)memchr((const void*)pszArgStart, '=', pszArgEnd - pszArgStart)) != NULL)
                {
                    const char *szNameEnd = pszValueStart;
                    while ((szNameEnd > pszArgStart) && isspace(szNameEnd[-1]))
                    {
                        szNameEnd --;
                    }
                    nLen = szNameEnd - pszArgStart;
                    pszValueStart ++;
                    while (pszValueStart < pszArgEnd && isspace(*pszValueStart))
                    {
                        pszValueStart ++;
                    }
                }
                if (pszValueStart && pszArgEnd && pszArgStart && nLen && pszValueStart < pszArgEnd)
                {
                    mapArgs[Decode(pszArgStart, nLen)] = Decode(pszValueStart, pszArgEnd - pszValueStart);
                }
            }
            return;
        }

        void CArgParserA::Reset()
        {
            mapArgs.clear();
        }


        std::wstring CArgParserW::GetArgString(const std::wstring& name, const std::wstring&def)
        {
            auto it = mapArgs.find(name);
            if (it == mapArgs.end())
            {
                return def;
            }
            if (it->second.empty())
            {
                return def;
            }
            return it->second;
        }

        __int64 CArgParserW::GetArgInt64(const std::wstring& name, __int64 defaut_value)
        {
            auto it = mapArgs.find(name);
            if (it == mapArgs.end())
            {
                return defaut_value;
            }
            if (it->second.empty())
            {
                return defaut_value;
            }
            return _wtoi64(it->second.c_str());
        }

        int CArgParserW::GetArgInt(const std::wstring& name, int defaut_value)
        {
            auto it = mapArgs.find(name);
            if (it == mapArgs.end())
            {
                return defaut_value;
            }
            if (it->second.empty())
            {
                return defaut_value;
            }
            return _wtoi(it->second.c_str());
        }

        bool CArgParserW::ArgExist(const std::wstring&strName)
        {
            auto it = mapArgs.find(strName);
            if (it == mapArgs.end())
            {
                return false;
            }
            return true;
        }

        bool CArgParserW::GetArgBool(const std::wstring& name, bool defaut_value)
        {
            auto it = mapArgs.find(name);
            if (it == mapArgs.end())
            {
                return defaut_value;
            }
            if (it->second.empty())
            {
                return defaut_value;
            }
            if (!_wcsicmp(it->second.c_str(), L"false")
                || it->second == L"0")
            {
                return false;
            }
            return true;
        }

        void CArgParserW::Parse(const std::wstring&strArg)
        {
            if (strArg.empty())
            {
                return;
            }
            const wchar_t* pszData = strArg.c_str();
            const wchar_t *pszArgStart = NULL;
            const wchar_t*pszArgEnd = NULL;
            const wchar_t *pszValueStart = NULL;
            const wchar_t*pszEnd = NULL;
            const wchar_t*pszNextArg = NULL;
            pszEnd = pszData + wcslen(pszData);
            pszNextArg = pszData;
            while (pszNextArg < pszEnd)
            {
                pszArgStart = pszNextArg;
                pszNextArg = wmemchr(pszArgStart,
                                                 '&',
                                                 pszEnd - pszArgStart);
                if (!pszNextArg)
                {
                    pszNextArg = pszEnd;
                }
                else
                {
                    pszNextArg ++;
                }
                pszArgEnd = pszNextArg;
                while (pszArgStart < pszArgEnd && iswspace(*pszArgStart))
                {
                    pszArgStart ++;
                }
                while ((pszArgEnd > pszArgStart) && (iswspace(pszArgEnd[-1]) || pszArgEnd[-1] == L'&'))
                {
                    pszArgEnd --;
                }
                if (pszArgStart >= pszArgEnd)
                {
                    continue;
                }
                int nLen = pszArgEnd - pszArgStart;
                if ((pszValueStart = wmemchr(pszArgStart, '=', pszArgEnd - pszArgStart)) != NULL)
                {
                    const wchar_t *szNameEnd = pszValueStart;
                    while ((szNameEnd > pszArgStart) && iswspace(szNameEnd[-1]))
                    {
                        szNameEnd --;
                    }
                    nLen = szNameEnd - pszArgStart;
                    pszValueStart ++;
                    while (pszValueStart < pszArgEnd && iswspace(*pszValueStart))
                    {
                        pszValueStart ++;
                    }
                }
                if (pszValueStart && pszArgEnd && pszArgStart && nLen && pszValueStart < pszArgEnd)
                {
                    mapArgs[Decode2W(pszArgStart, nLen)] = Decode2W(pszValueStart, pszArgEnd - pszValueStart);
                }
            }
            return;
        }

        void CArgParserW::Reset()
        {
            mapArgs.clear();
        }

    };

    namespace rot13
    {
        std::wstring Encode(const wchar_t* pszInput)
        {
            if (!pszInput)
            {
                return L"";
            }
            wstring strOutput;
            for (LPCWSTR pszItem = pszInput; *pszItem; pszItem ++)
            {
                wchar_t c = *pszItem;
                if ('a' <= c && c <= 'z')
                {
                    if (c > 'm')
                    {
                        c -= 13;
                    }
                    else
                    {
                        c += 13;
                    }
                }
                else if ('A' <= c && c <= 'Z')
                {
                    if (c > 'M')
                    {
                        c -= 13;
                    }
                    else
                    {
                        c += 13;
                    }
                }
                strOutput.append(&c, 1);
            }
            return strOutput;
        }

        std::wstring Decode(const wchar_t* pszInput)
        {
            return Encode(pszInput);
        }

    };

}
