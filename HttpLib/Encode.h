#pragma once
#include "bddebug.h"
#include <string>
#include <map>
#include <unordered_map>
#include <algorithm>
using namespace std;
#define A2B(c)	((c >= L'0' && c <= L'9') ? c-L'0': \
	(c >= L'a' && c <= L'z') ? c-L'a' + 10: \
	(c >= L'A' && c <= L'Z') ? c-L'A' + 10: 17 \
	)

namespace Util
{    
    namespace StringUtility
    {
        struct Hash_Key
        {
            size_t operator()(const std::wstring& key) const
            {
                std::wstring strTmp(key);
                struct TOLOW
                {
                    void operator()(wchar_t& ch)
                    {
                        if ((ch >= (wchar_t)L'A') && (ch <= (wchar_t)L'Z'))
                        {
                            ch = (wchar_t)(ch - L'A' + L'a');
                        }
                    }
                };
                std::for_each(strTmp.begin(), strTmp.end(), TOLOW());
                typedef std::hash<std::wstring> HASH;
                return HASH()(strTmp);
            }
        };

        struct Equal_Key
        {
            bool operator()(const std::wstring& left, const std::wstring& right) const
            {
                return !_wcsicmp(left.c_str(), right.c_str());
            }
        };
        struct Hash_KeyA
        {
            size_t operator()(const std::string& key) const
            {
                std::string strTmp(key);
                struct TOLOW
                {
                    void operator()(char& ch)
                    {
                        if ((ch >= (char)'A') && (ch <= (char)'Z'))
                        {
                            ch = (char)(ch - 'A' + 'a');
                        }
                    }
                };
                std::for_each(strTmp.begin(), strTmp.end(), TOLOW());
                typedef std::hash<std::string> HASH;
                return HASH()(strTmp);
            }
        };

        struct Equal_KeyA
        {
            bool operator()(const std::string& left, const std::string& right) const
            {
                return !_stricmp(left.c_str(), right.c_str());
            }
        };

    }
}
namespace Util
{
    namespace Code
    {
        std::wstring Md5BinW(const unsigned char*pbData, size_t size);
        std::string Md5Bin(const unsigned char*pbData, size_t size);        
    };
    namespace Md5
    {
        std::wstring Encode2W(const unsigned char*pbData, size_t size);
        std::string Encode(const unsigned char*pbData, size_t size);
        std::string Encode(const std::string&);
    }
    namespace Hex
    {
        std::string Encode(const LPBYTE data, DWORD dlen);
        std::wstring EncodeW(PBYTE data, DWORD dlen);
        std::string DecodeW(const WCHAR*psz, size_t uLen);
        std::string Decode(const char*psz, size_t uLen);
        std::string Decode(const std::string&);
    };
    namespace rot13
    {
        wstring Encode(const wchar_t* pszInput);
        wstring Decode(const wchar_t* pszInput);
    }
    namespace URL
    {
        wstring EncodeW(const wchar_t*, size_t, DWORD codepage = CP_UTF8);
        wstring EncodeW2W(const wchar_t*, size_t, DWORD codepage = CP_UTF8);
        string Encode(const char* lpszVal, size_t lenVal);
        string Encode(const std::string&strValue);

        wstring Encode2W(const char* lpszVal, size_t lenVal);
        string Encode(const wchar_t* lpszVal, size_t lenVal, DWORD codepage = CP_UTF8);

        string Decode(const char* lpszVal, size_t lenVal);
        string Decode2(const wchar_t* lpszVal, size_t lenVal);
        wstring Decode2W(const wchar_t* lpszVal, size_t lenVal, DWORD codepage = CP_UTF8);
        wstring DecodeW(const char* lpszVal, size_t lenVal, DWORD codepage = CP_UTF8);

        void Parse(const std::string &url
                   , std::string*pstrScheme
                   , std::string*pstrHost
                   , std::string*pstrPath
                   , int*pnPort
                   , std::string*pstrArg);
        void ParseW(const std::wstring &url
                    , std::wstring*pstrScheme
                    , std::wstring*pstrHost
                    , std::wstring*pstrPath
                    , int*pnPort
                    , std::wstring*pstrArg);

        
        class CArgParserA
        {
        public:
            std::string GetArgString(const std::string& name, const std::string&def);
            __int64 GetArgInt64(const std::string& name, __int64 defaut_value);
            int GetArgInt(const std::string& name, int defaut_value);
            bool ArgExist(const std::string&);
            bool GetArgBool(const std::string& name, bool defaut_value);
            void Parse(const std::string&);
            void Reset();
        private:
            std::unordered_map<std::string, std::string, 
                Util::StringUtility::Hash_KeyA, 
                Util::StringUtility::Equal_KeyA> mapArgs;
        };
        class CArgParserW
        {
        public:
            std::wstring GetArgString(const std::wstring& name, const std::wstring&def);
            __int64 GetArgInt64(const std::wstring& name, __int64 defaut_value);
            int GetArgInt(const std::wstring& name, int defaut_value);
            bool ArgExist(const std::wstring&);
            bool GetArgBool(const std::wstring& name, bool defaut_value);
            void Parse(const std::wstring&);
            void Reset();
        private:
            std::unordered_map<std::wstring, std::wstring, 
                Util::StringUtility::Hash_Key, 
                Util::StringUtility::Equal_Key> mapArgs;
        };

        class CParser
        {
        public:
            wstring GetArgString(const wchar_t* name, const wchar_t*def);
            __int64 GetArgInt64(const wchar_t* name, __int64 defaut_value);
            int GetArgInt(const wchar_t* name, int defaut_value);
            bool ArgExist(const wstring&);
            wstring GetPath();
            bool GetArgBool(const wchar_t* name, bool defaut_value);
            void CrackUrl(LPCWSTR pszURL);
            void InitFields();
        private:
            wstring m_strPath;
            wstring m_strScheme;
            CArgParserW m_argParser;
        };
    };
}

