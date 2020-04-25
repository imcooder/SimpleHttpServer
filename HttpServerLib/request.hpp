//
// request.hpp
// ~~~~~~~~~~~
//
// Copyright (c) 2003-2012 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef HTTP_SERVER3_REQUEST_HPP
#define HTTP_SERVER3_REQUEST_HPP

#include <string>
#include <vector>
#include <map>
#include <unordered_map>
using namespace std;
#include "header.hpp"
#include <boost/algorithm/string.hpp>
namespace http
{
	namespace server3 
	{
		struct Hash_Key
		{ 
			size_t operator()(const std::string &key) const
			{ 
				std::string strTmp(key);
				boost::to_lower(strTmp);
				typedef std::hash<std::string> HASH;
				return HASH()(strTmp); 
			}
		};

		struct Equal_Key
		{
			bool operator()(const std::string &left, const std::string &right) const
			{
				return !_stricmp(left.c_str(), right.c_str());
			}
		};
		/// A request received from a client.		
		struct request
		{
			std::string method;
			std::string uri;
			string path;
			int http_version_major;
			int http_version_minor;
			std::vector<header> headers;
			std::string post_content;
			std::unordered_map<string, string, http::server3::Hash_Key, http::server3::Equal_Key> mapArgs;
		public:
			string GetString(const char* name, const char*def)
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
			wstring GetWString(const char* name, const wchar_t*def)
			{
				WCHAR szBuffer[2049] = {0};
				auto it = mapArgs.find(name);
				if (it == mapArgs.end())
				{
					return def;
				}
				if (it->second.empty())
				{
					return def;
				}
				int _Dsize = MultiByteToWideChar(CP_UTF8, 0, it->second.c_str(), it->second.length(), NULL, 0);
				if (_Dsize < _countof(szBuffer) - 1)
				{
					MultiByteToWideChar(CP_UTF8, 0, it->second.c_str(), it->second.length(), szBuffer, _countof(szBuffer) - 1);
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
					MultiByteToWideChar(CP_UTF8, 0, it->second.c_str(), it->second.length(), _Dest, _Dsize);
					_Dest[_Dsize] = 0;
					return _Dest;
				}
			}
			__int64 GetInt64(const char* name, __int64 defaut_value)
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
			int GetInt(const char* name, int defaut_value)
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
				return atoi(it->second.c_str());				
			}
            int GetBool(const char* name, bool defaut_value)
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
            std::string GetHeader(const std::string&strKey, const std::string&strDefault)
            {
                for (auto it = headers.begin(); it != headers.end(); ++it)
                {
                    if (!_stricmp(it->name.c_str(), strKey.c_str()))
                    {
                        return it->value;
                    }
                }
                return strDefault;
            }
		};

	} // namespace server3
} // namespace http

#endif // HTTP_SERVER3_REQUEST_HPP
