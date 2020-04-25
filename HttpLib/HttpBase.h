#pragma once
#include <string>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <unordered_map>
#include "http.h"

#ifndef NET_DBG
#define NET_DBG(...)  XLogDebug( XTAG(L"net"), __VA_ARGS__);
#endif
#ifndef NET_ERR
#define NET_ERR(...)  XLogError( XTAG(L"net"), __VA_ARGS__);
#endif

namespace http
{
#define HTTP_CONNECT_TYPE_APPLICATION ("application/x-www-form-urlencoded")
#define HTTP_CONNECT_TYPE_JSON	    	("application/json;charset=utf-8")
#define HTTP_CONNECT_TYPE_HTML	    	("text/html")

    class CHttpBase
    {
    public:
        CHttpBase();
        virtual ~CHttpBase();
        void SetConnectTimeOut(int timeout);
        void SetResolveTimeout(int timeout);
        void SetWriteTimeout(int timeout);
        void SetReadTimeout(int timeout);
        bool Get(const std::string &url, const std::string &type );
        bool Post(const std::string &url, const std::string &post, const std::string &type);
        std::string GetResult();
        std::string GetResponseHeader(const std::string&name, const std::string&def);
        http::HEADER_MAP GetResponseHeader();
        std::string GetLastError();
        int GetErrorCode();
        void SetLastError(const std::string&);
        void SetErrorCode(int nValue);
        void SetCookie(const char*pszValue);
        void SetReferer(const char*pszValue);
        void SetContentEncode(const std::string&);
        void SetAcceptEncode(const std::string&value);
        void SetAccept(const std::string& value);
        void SetUserAgent(const std::string&value);
        void SetProxy(const std::string&strProxy, const std::string& nPort);
    private:
        std::string m_result;
        http::HEADER_MAP m_mapResponseHeader;
        std::string m_strError;
        int m_nErrorCode;
        int m_nConnectTimeout;
        int m_nResolveTimeout;
        int m_nWriteTimeout;
        int m_nReadTimeout;
        std::string m_strCookie;
        std::string m_strReferer;
        std::string m_strContentEncode;
        std::string m_strAcceptEncode;
        std::string m_strAccept;
        std::string m_strUserAgent;
        std::string m_strProxyServer;
        std::string m_strProxyPort;
    };
};